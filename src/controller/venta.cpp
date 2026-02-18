#include "controller/venta.hpp"

Venta::Venta(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : BVentaPago(cobject, refBuilder),
                                                                                      faltante(0)
{
    v_lbl_titulo->set_text("Venta");
    v_btn_timeout_retry->set_visible(false);

    async_gui.dispatcher.connect(sigc::mem_fun(async_gui, &Global::Async::on_dispatcher_emit));

    CROW_ROUTE(RestApp::app, "/accion/inicia_venta").methods("POST"_method)(sigc::mem_fun(*this, &Venta::inicia));
    CROW_ROUTE(RestApp::app, "/accion/detiene_venta").methods("GET"_method)(sigc::mem_fun(*this, &Venta::deten));

    CROW_WEBSOCKET_ROUTE(RestApp::app, "/ws/venta")
        .onopen(sigc::mem_fun(*this, &Venta::on_wb_socket_open))
        .onclose(sigc::mem_fun(*this, &Venta::on_wb_socket_close))
        .onmessage(sigc::mem_fun(*this, &Venta::on_wb_socket_message));
}

Venta::~Venta()
{
}

void Venta::on_wb_socket_open(crow::websocket::connection &conn)
{
    std::cout << "WebSocket connected: " << conn.get_remote_ip() << std::endl;
    connection = &conn;

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Esperar a que el cliente esté listo para recibir mensajes
    crow::json::wvalue response;
    response["ingreso"] = Global::EValidador::balance.ingreso.load();
    response["cambio"] = Global::EValidador::balance.cambio.load();
    response["total"] = Global::EValidador::balance.total.load();
    response["terminado"] = !Global::EValidador::is_running.load();
    response["status"] = Global::EValidador::is_running.load() ? "En proceso" : "Proceso terminado";
    response["faltante"] = faltante;
    
    conn.send_text(response.dump());

}

void Venta::on_wb_socket_close(crow::websocket::connection &conn, const std::string &reason, uint16_t code)
{
    if (connection == &conn)
        connection = nullptr; // Limpiamos la referencia a la conexión cerrada
    std::cout << "WebSocket disconnected: " << conn.get_remote_ip() << " Reason: " << reason << " Code: " << code << std::endl;
}

void Venta::on_wb_socket_message(crow::websocket::connection &conn, const std::string &data, bool is_binary)
{

    auto json_data = crow::json::load(data);

    if (json_data["action"] == "detener")
    {
        on_btn_cancel_click();
        conn.send_text(R"({"status":"detenido"})");
    }
    
}

void Venta::on_btn_retry_click()
{
    std::cout << "Click otra vez desde Venta\n";
}

void Venta::on_btn_cancel_click()
{
    Global::EValidador::is_running.store(false);
    cancelado = true;
    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });
}

crow::response Venta::deten(const crow::request &req)
{
    on_btn_cancel_click();
    return crow::response(200, "Venta detenida");
}

void Venta::func_poll(const std::string &status, const crow::json::rvalue &data)
{
    using namespace Global::EValidador;

    if (connection)
    {
        crow::json::wvalue response;
        response["ingreso"] = Global::EValidador::balance.ingreso.load();
        response["cambio"] = Global::EValidador::balance.cambio.load();
        response["total"] = Global::EValidador::balance.total.load();
        response["terminado"] = !Global::EValidador::is_running.load();
        response["status"] = Global::EValidador::is_running.load() ? "En proceso" : "Proceso terminado";
        response["faltante"] = faltante;
        
        connection->send_text(response.dump());
    }

    if (status == "ESCROW")
    {
        auto bill_selection = Device::dv_bill.get_level_cash_actual(true);
        for (size_t i = 0; i < bill_selection->get_n_items(); i++)
        {
            auto m_list = bill_selection->get_typed_object<MLevelCash>(i);
            if (m_list->m_denominacion == (data["value"].i() / 100))
                if (m_list->m_cant_recy < m_list->m_nivel_inmo_max)
                    Device::dv_bill.acepta_dinero(m_list->m_denominacion, true);
                else // if (m_list->m_cant_recy > m_list->m_nivel_inmo_max)
                    Device::dv_bill.acepta_dinero(m_list->m_denominacion, false);
        }
    }

    if (status == "COIN_CREDIT" ||
        status == "VALUE_ADDED" ||
        status == "ESCROW")
    {

        balance.ingreso.store(balance.ingreso.load() + (data["value"].i() / 100));
        async_gui.dispatch_to_gui([this]()
        {
            v_lbl_recibido->set_text(std::to_string(balance.ingreso.load()));

            faltante = balance.ingreso.load() > balance.total.load() ? 0 : balance.total.load() - balance.ingreso.load();
            balance.cambio.store(balance.ingreso.load() > balance.total.load() ? balance.ingreso.load() - balance.total.load() : 0);

            v_lbl_cambio->set_text(std::to_string(balance.cambio.load()));
            v_lbl_faltante->set_text(std::to_string(faltante)); 
        });

        if (balance.ingreso.load() >= balance.total.load())
        {
            Global::EValidador::is_running.store(false);
            Device::dv_coin.deten_cobro_v6();
            Device::dv_bill.deten_cobro_v6();
        }
    }
}

crow::response Venta::inicia(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Venta);

    using namespace Global::EValidador;
    auto bodyParams = crow::json::load(req.body);
    cancelado = false;
    estatus.clear();
    faltante = bodyParams["value"].i();

    auto snapshot_inicial_bill = Device::dv_bill.get_level_cash_actual(true, false);
    auto snapshot_inicial_coin = Device::dv_coin.get_level_cash_actual(true, false);

    bool is_view_ingreso = bodyParams.has("is_view_ingreso") && bodyParams["is_view_ingreso"].b();
    is_view_ingreso ? v_lbl_titulo->set_text("Ingreso") : v_lbl_titulo->set_text("Venta");

    std::string concepto = bodyParams["concepto"].operator std::string();
    balance.total.store(faltante);
    balance.ingreso.store(0);
    balance.cambio.store(0);
    Pago::faltante = 0;

    is_running.store(true);

    async_gui.dispatch_to_gui([this, bodyParams]()
    { 
        auto s_total = std::to_string(bodyParams["value"].i());
        Global::Widget::v_main_stack->set_visible_child(*this); 
        v_lbl_monto_total->set_text(s_total);
        v_lbl_faltante->set_text(s_total);
        v_lbl_cambio->set_text("0");
        v_lbl_recibido->set_text("0"); 
    });

    Device::dv_bill.inicia_dispositivo_v6();
    Device::dv_coin.inicia_dispositivo_v6();

    auto bill_selection = Device::dv_bill.get_level_cash_actual(true);
    for (size_t i = 0; i < bill_selection->get_n_items(); i++)
    {
        auto m_list = bill_selection->get_typed_object<MLevelCash>(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Simulate some delay for the device to be ready

        if (m_list->m_cant_recy < m_list->m_nivel_inmo_max)
            Device::dv_bill.acepta_dinero(m_list->m_denominacion, true);
        else // if (m_list->m_cant_recy > m_list->m_nivel_inmo_max)
            Device::dv_bill.acepta_dinero(m_list->m_denominacion, false);
    }

    auto future1 = std::async(std::launch::async, [this](){ Device::dv_coin.poll(sigc::mem_fun(*this, &Venta::func_poll)); });
    auto future2 = std::async(std::launch::async, [this](){ Device::dv_bill.poll(sigc::mem_fun(*this, &Venta::func_poll)); });

    future1.wait();
    future2.wait();

    auto final_bill = Device::dv_bill.get_level_cash_actual(true, false);
    auto final_coin = Device::dv_coin.get_level_cash_actual(true, false);

    if (cancelado)
    {
        estatus = "Operación cancelada";
        if (balance.ingreso.load() > 0)
            Pago::da_pago(balance.ingreso.load(), is_view_ingreso ? "Ingreso" : "Venta", estatus);
    }
    else if (balance.cambio.load() > 0)
        Pago::da_pago(balance.cambio.load(), is_view_ingreso ? "Ingreso" : "Venta", estatus);
    if (Pago::faltante > 0)
        estatus = "Cambio Incompleto, faltante: " + std::to_string(Pago::faltante);

    auto diff_bill = DetalleMovimiento::calcular_diferencias_niveles(snapshot_inicial_bill, final_bill);
    auto diff_coin = DetalleMovimiento::calcular_diferencias_niveles(snapshot_inicial_coin, final_coin);

    auto detalle_store = Gio::ListStore<MDetalleMovimiento>::create();

    crow::json::wvalue data;
    Log log;
    auto t_log = MLog::create(
        0,
        Global::User::id,
        is_view_ingreso ? "Ingreso" : "Venta",
        concepto.empty() ? "Sin Concepto" : concepto,
        balance.ingreso.load(),
        balance.cambio.load(),
        balance.total.load(),
        not Global::Utility::is_ok || cancelado ? estatus : "Exito.",
        Glib::DateTime::create_now_local());

    t_log->m_id = log.insert_log(t_log);
    t_log->m_estatus = (not Global::Utility::is_ok || cancelado ? estatus : "Exito.");
    Global::Utility::is_ok = true;

    for (const auto& [denom, qty] : diff_bill)
    {
        auto detalle = MDetalleMovimiento::create(0, t_log->m_id, "entrada", denom, qty);
        detalle_store->append(detalle);
    }
    for (const auto& [denom, qty] : diff_coin)
    {
        auto detalle = MDetalleMovimiento::create(0, t_log->m_id, "entrada", denom, qty);
        detalle_store->append(detalle);
    }

    for (const auto& [denom, qty] : Pago::salidas_totales)
    {
        auto detalle = MDetalleMovimiento::create(0, t_log->m_id, "salida", denom, qty);
        detalle_store->append(detalle);
    }

    Pago::salidas_totales.clear();

    auto bd_detalle = std::make_unique<DetalleMovimiento>();
    bd_detalle->insertar_detalle_movimiento(t_log->m_id, detalle_store);

    if (Global::Widget::Impresora::v_switch_impresion->get_active())
    {
        std::string command = "echo \"" + Global::System::imprime_ticket(t_log, faltante) + "\" | lp";
        std::system(command.c_str());
    }

    data = Global::Utility::json_ticket(t_log);
    data["Cambio_faltante"] = Pago::faltante;
    
    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();

    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });

    return crow::response(data);
}

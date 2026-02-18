#include "controller/pago.hpp"
#include "pago.hpp"
#include "carrousel.hpp"

Pago::Pago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : BVentaPago(cobject, refBuilder)
{
    v_lbl_titulo->set_text("Pago");
    v_lbl_timeout->set_visible(false);
    v_BXRW4->set_visible(false);

    async_gui.dispatcher.connect(sigc::mem_fun(async_gui, &Global::Async::on_dispatcher_emit));

    CROW_ROUTE(RestApp::app, "/accion/inicia_pago").methods("POST"_method)(sigc::mem_fun(*this, &Pago::inicia));
    CROW_ROUTE(RestApp::app, "/accion/inicia_pago_manual").methods("POST"_method)(sigc::mem_fun(*this, &Pago::inicia_manual));

    CROW_ROUTE(RestApp::app, "/accion/inicia_cambio").methods("POST"_method)(sigc::mem_fun(*this, &Pago::inicia_cambio));
    CROW_ROUTE(RestApp::app, "/accion/inicia_cambio_manual").methods("POST"_method)(sigc::mem_fun(*this, &Pago::inicia_cambio_manual));
    CROW_ROUTE(RestApp::app, "/accion/termina_cambio_manual").methods("POST"_method)(sigc::mem_fun(*this, &Pago::termina_cambio_manual));
    CROW_ROUTE(RestApp::app, "/accion/cancelar_cambio_manual").methods("POST"_method)(sigc::mem_fun(*this, &Pago::cancelar_cambio_manual));
}

Pago::~Pago()
{
}

void Pago::on_btn_retry_click()
{
    std::cout << "Click otra vez desde Pago\n";
}

void Pago::on_btn_cancel_click()
{
    std::cout << "Click cancela desde Pago\n";
}

void Pago::poll_cambio(const std::string &status, const crow::json::rvalue &data)
{
    if (status == "COIN_CREDIT" ||
        status == "VALUE_ADDED" ||
        status == "ESCROW")
    {
        Global::EValidador::balance.ingreso.store((data["value"].i() / 100));

        Global::EValidador::is_running.store(false);
        Device::dv_coin.deten_cobro_v6();
        Device::dv_bill.deten_cobro_v6();
    }

}

void Pago::poll_pago(const std::pair<int, std::string> &status)
{
    auto json_data = crow::json::load(status.second);
    if (std::string data = json_data["dispenseResult"].s(); json_data.has("dispenseResult"))
        if (data == "COMPLETED")
        {
            Global::System::showNotify("Pago", "Pago completado con éxito", "dialog-information");
        }
        else if (data == "IN_PROGRESS")
        {
            /*continue*/
        }
        else if (data == "ERROR")
        {
            Global::System::showNotify("Pago", "Error al procesar el pago: Error en validador", "dialog-error");
        }
        else if (data == "TIME_OUT")
        {
            Global::System::showNotify("Pago", "Pago Incompleto", "dialog-warning");
        }
}

void Pago::da_pago(int cambio, const std::string &tipo, std::string &estatus)
{
    Pago::faltante = 0;
    auto s_level_mon = Device::map_cantidad_recyclador(Device::dv_coin);
    auto s_level_bill = Device::map_cantidad_recyclador(Device::dv_bill);

    auto r_bill = Global::Utility::obten_cambio(cambio, s_level_bill);
    auto r_coin = Global::Utility::obten_cambio(cambio, s_level_mon);

    if (cambio > 0)
    {
        Pago::faltante = cambio;
        Global::Utility::is_ok = false;
        Global::System::showNotify(tipo.c_str(), "No se cuenta con suficiente efectivo", "dialog-error");
    }
    
    da_pago(r_bill.dump(), r_coin.dump(), tipo, estatus);
}

void Pago::da_pago(int cambio, const std::string &tipo, std::string &estatus, bool is_cambio)
{
    Pago::faltante = 0;
    auto s_level_mon = Device::map_cantidad_recyclador(Device::dv_coin);
    auto s_level_bill = Device::map_cantidad_recyclador(Device::dv_bill);

    auto r_bill = Global::Utility::obten_cambio(cambio, s_level_bill, is_cambio);
    auto r_coin = Global::Utility::obten_cambio(cambio, s_level_mon, is_cambio);

    if (cambio > 0)
    {
        Pago::faltante = cambio;
        Global::Utility::is_ok = false;
        Global::System::showNotify(tipo.c_str(), "No se cuenta con suficiente efectivo", "dialog-error");
    }
    
    da_pago(r_bill.dump(), r_coin.dump(), tipo, estatus);
}

void Pago::da_pago(const std::string &bill, const std::string &coin, const std::string &tipo, std::string &estatus)
{
    std::pair<int, std::string> status_bill = {200,""}, status_coin = {200,""};
    int max_intentos = 0;

    if (bill == "[0,0,0,0,0,0]" && coin == "[0,0,0,0]")
    {
        Global::Utility::is_ok = false;
        Global::System::showNotify(tipo.c_str(), "No se cuenta con suficiente efectivo", "dialog-error");
        Pago::faltante = Global::EValidador::balance.cambio.load();
    }
    else
    {
        auto inicial_bill = Device::dv_bill.get_level_cash_actual(true, false);
        auto inicial_coin = Device::dv_coin.get_level_cash_actual(true, false);
        // Procesamiento para dv_bill
        if (bill != "[0,0,0,0,0,0]")
        {
            Device::dv_bill.inicia_dispositivo_v6();

            status_bill = Device::dv_bill.reintenta_comando_post("PayoutMultipleDenominations", bill, max_intentos);
            if (max_intentos > 9)
            {
                Device::dv_bill.deten_cobro_v6();
                std::this_thread::sleep_for(std::chrono::seconds(3));
                Device::dv_bill.inicia_dispositivo_v6();
                status_bill = Device::dv_bill.reintenta_comando_post("PayoutMultipleDenominations", bill, max_intentos);
            }

            poll_pago(status_bill);
        }

        max_intentos = 0;

        // Procesamiento para dv_coin
        if (coin != "[0,0,0,0]")
        {
            Device::dv_coin.inicia_dispositivo_v6();

            status_coin = Device::dv_coin.reintenta_comando_post("PayoutMultipleDenominations", coin , max_intentos);
            if (max_intentos > 9)
            {
                Device::dv_coin.deten_cobro_v6();
                std::this_thread::sleep_for(std::chrono::seconds(3));
                Device::dv_coin.inicia_dispositivo_v6();
                status_coin = Device::dv_coin.reintenta_comando_post("PayoutMultipleDenominations", coin , max_intentos);
            }
            poll_pago(status_coin);
        }

        std::this_thread::sleep_for(std::chrono::seconds(2)); // Esperar a que los dispositivos actualicen su estado

        auto final_bill = Device::dv_bill.get_level_cash_actual(true, false);
        auto final_coin = Device::dv_coin.get_level_cash_actual(true, false);

        auto salidas_bill = DetalleMovimiento::calcular_diferencias_niveles(inicial_bill, final_bill);
        auto salidas_coin = DetalleMovimiento::calcular_diferencias_niveles(inicial_coin, final_coin);

        for (const auto& p : salidas_bill) salidas_totales[p.first] += p.second;
        for (const auto& p : salidas_coin) salidas_totales[p.first] += p.second;

        if (status_bill.first != crow::status::OK || status_coin.first != crow::status::OK)
        {
            estatus = "Error al procesar el pago";
            Global::System::showNotify(tipo.c_str(), estatus.c_str(), "dialog-error");
        }
    }
    
}

crow::response Pago::inicia(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Cambio_A);
    using namespace Global::EValidador;
    auto bodyParams = crow::json::load(req.body);
    Global::Utility::is_ok = true;
    Pago::faltante = 0;

    int cambio = balance.cambio = bodyParams["value"].i();
    std::string concepto = bodyParams["concepto"].operator std::string();

    if (cambio <= 0)
        return crow::response("Nada que devolver");

    balance.total.store(bodyParams["value"].i());
    is_running.store(true);
    balance.ingreso.store(0);
    
    async_gui.dispatch_to_gui([this, cambio]()
    { 
        auto s_total = Glib::ustring::compose("%1",cambio);
        
        Global::Widget::v_main_stack->set_visible_child(*this); 
        v_lbl_monto_total->set_text(s_total);
        v_lbl_faltante->set_text(s_total);
        v_lbl_recibido->set_text("0"); 
    });

    Pago::da_pago(balance.cambio.load(), "Pago", estatus);

    Log log;
    crow::json::wvalue data;
    auto t_log = MLog::create(
        0,
        Global::User::id,
        "Pago",
        concepto.empty() ? "Sin Concepto" : concepto,
        0,
        balance.cambio.load(),
        0,
        not Global::Utility::is_ok ? estatus : "Pago Realizada con Exito.",
        Glib::DateTime::create_now_local());

    t_log->m_id = log.insert_log(t_log);

    Global::Utility::is_ok = true;

    if (Global::Widget::Impresora::v_switch_impresion->get_active())
    {
        std::string command = "echo \"" + Global::System::imprime_ticket(t_log, faltante) + "\" | lp";
        std::system(command.c_str());
    }

    data = Global::Utility::json_ticket(t_log);
    data["Cambio_faltante"] = Pago::faltante;

    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();

    async_gui.dispatch_to_gui([this]() { Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });

    balance.ingreso.store(0);
    balance.cambio.store(0);

    return crow::response(data);
}

crow::response Pago::inicia_manual(const crow::request &req) {
    estatus.clear();
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Cambio_M);
    auto bodyParams = crow::json::load(req.body);
    auto [cambio, bill_values, coin_values] = procesar_parametros_iniciales(bodyParams);

    auto snapshot_inicial_bill = Device::dv_bill.get_level_cash_actual(true, false);
    auto snapshot_inicial_coin = Device::dv_coin.get_level_cash_actual(true, false);

    validar_inventario_disponible(bill_values, coin_values);
    configurar_estado_pago(cambio);
    mostrar_interfaz_pago_manual(calcular_total_pago(bill_values, coin_values));

    auto t_log = registrar_pago_y_log(bill_values, coin_values, "Pago Manual");

    auto final_bill = Device::dv_bill.get_level_cash_actual(true, false);
    auto final_coin = Device::dv_coin.get_level_cash_actual(true, false);

    auto diff_bill = DetalleMovimiento::calcular_diferencias_niveles(snapshot_inicial_bill, final_bill);
    auto diff_coin = DetalleMovimiento::calcular_diferencias_niveles(snapshot_inicial_coin, final_coin);

    auto detalle_store = Gio::ListStore<MDetalleMovimiento>::create();

        for (const auto& [denom, qty] : Pago::salidas_totales)
    {
        auto detalle = MDetalleMovimiento::create(0, t_log->m_id, "salida", denom, qty);
        detalle_store->append(detalle);
    }

    Pago::salidas_totales.clear();

    auto bd_detalle = std::make_unique<DetalleMovimiento>();
    bd_detalle->insertar_detalle_movimiento(t_log->m_id, detalle_store);

    imprimir_ticket_si_corresponde(t_log);

    return finalizar_proceso_pago(t_log);
}

// Funciones auxiliares factorizadas:

std::tuple<int, std::vector<int>, std::vector<int>> Pago::procesar_parametros_iniciales(const crow::json::rvalue& bodyParams) {
    using namespace Global::EValidador;
    int cambio = balance.cambio = bodyParams["total"].i();
    concepto = bodyParams["concepto"].operator std::string();
    Global::Utility::is_ok = true;
    Pago::faltante = 0;
    balance.total = cambio;

    std::vector<int> bill_values;
    for (auto&& i : bodyParams["bill"])
        bill_values.push_back(i.i());

    std::vector<int> coin_values;
    for (auto&& i : bodyParams["coin"])
        coin_values.push_back(i.i());

    return {cambio, bill_values, coin_values};
}

void Pago::validar_inventario_disponible(const std::vector<int>& bill_values, const std::vector<int>& coin_values) {
    auto s_level_mon = Device::map_cantidad_recyclador(Device::dv_coin);
    auto s_level_bill = Device::map_cantidad_recyclador(Device::dv_bill);

    for (size_t i = 0; i < s_level_mon.size(); i++) {
        if (coin_values[i] > s_level_mon.at(map_coin.at(i)))
            throw std::runtime_error("No hay suficiente cambio en la denominacion: " + std::to_string(map_coin.at(i)) + " de monedas");
    }

    for (size_t i = 0; i < s_level_bill.size(); i++) {
        if (bill_values[i] > s_level_bill.at(map_bill.at(i)))
            throw std::runtime_error("No hay suficiente cambio en la denominacion: " + std::to_string(map_bill.at(i)) + " de billetes");
    }
}

void Pago::configurar_estado_pago(int cambio) {
    using namespace Global::EValidador;

    is_running.store(true);
    balance.ingreso.store(0);
    
    balance.total = cambio;
}

int Pago::calcular_total_pago(const std::vector<int>& bill_values, const std::vector<int>& coin_values) {
    int total = 0;
    for (size_t i = 0; i < coin_values.size(); i++)
        total += coin_values[i] * map_coin.at(i);
    
    for (size_t i = 0; i < bill_values.size(); i++)
        total += bill_values[i] * map_bill.at(i);
    
    return total;
}

void Pago::mostrar_interfaz_pago_manual(int total) {
    async_gui.dispatch_to_gui([this, total]() {
        Global::Widget::v_main_stack->set_visible_child(*this); 
        v_lbl_monto_total->set_text("Monto Manual: " + std::to_string(total));
        v_lbl_faltante->set_text("0");
        v_lbl_recibido->set_text("0"); 
    });
}

Glib::RefPtr<MLog> Pago::registrar_pago_y_log(const std::vector<int>& bill_values, const std::vector<int>& coin_values, const std::string& tipo_pago) {
    std::string bill_str = vector_to_json_array(bill_values);
    std::string coin_str = vector_to_json_array(coin_values);

    auto s_level_mon = Device::map_cantidad_recyclador(Device::dv_coin);
    auto s_level_bill = Device::map_cantidad_recyclador(Device::dv_bill);
    const auto total_ant_coin = Global::Utility::total_anterior(s_level_mon);
    const auto total_ant_bill = Global::Utility::total_anterior(s_level_bill);

    Pago::da_pago(bill_str, coin_str, tipo_pago, estatus);

    Log log;
    auto t_log = MLog::create(
        0,
        Global::User::id,
        tipo_pago,
        concepto,
        0,
        calcular_total_pago(bill_values, coin_values),
        0,
        not Global::Utility::is_ok ? estatus : "Exito.",
        Glib::DateTime::create_now_local());
    
    Global::Utility::is_ok = true;
    t_log->m_id = log.insert_log(t_log);
    
    return t_log;
}

void Pago::imprimir_ticket_si_corresponde(const Glib::RefPtr<MLog>& t_log) {
    if (Global::Widget::Impresora::v_switch_impresion->get_active()) {
        std::string command = "echo \"" + Global::System::imprime_ticket(t_log, faltante) + "\" | lp";
        std::system(command.c_str());
    }
}

crow::response Pago::finalizar_proceso_pago(const Glib::RefPtr<MLog>& t_log) {
    t_log->m_estatus = (not Global::Utility::is_ok ? estatus : "Exito.");
    crow::json::wvalue data = Global::Utility::json_ticket(t_log);
    data["Cambio_faltante"] = Pago::faltante;

    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();

    async_gui.dispatch_to_gui([](){ Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });

    return crow::response(data);
}

std::string Pago::vector_to_json_array(const std::vector<int>& values) {
    std::string result = "[";
    for (size_t i = 0; i < values.size(); i++)
        result += std::to_string(values[i]) + ",";
    if (!values.empty()) result.pop_back();
    result += "]";
    return result;
}


crow::response Pago::inicia_cambio(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Cambio_A);
    using namespace Global::EValidador;
    auto bodyParams = crow::json::load(req.body);
    Global::Utility::is_ok = true;
    Pago::faltante = 0;
    is_running.store(true);

    Device::dv_coin.inicia_dispositivo_v6();
    Device::dv_bill.inicia_dispositivo_v6();

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

    auto snapshot_inicial_bill = Device::dv_bill.get_level_cash_actual(true, false);
    auto snapshot_inicial_coin = Device::dv_coin.get_level_cash_actual(true, false);

    auto future1 = std::async(std::launch::async, [this](){ Device::dv_coin.poll(sigc::mem_fun(*this, &Pago::poll_cambio)); });
    auto future2 = std::async(std::launch::async, [this](){ Device::dv_bill.poll(sigc::mem_fun(*this, &Pago::poll_cambio)); });

    future1.wait();
    future2.wait();

    int cambio = balance.ingreso.load();

    std::string concepto = bodyParams["concepto"].operator std::string();
    
    if (cambio <= 0)
        return crow::response("Nada que devolver");
    
    balance.ingreso.store(0);
    

    async_gui.dispatch_to_gui([this, cambio]()
    { 
        auto s_total = std::to_string(cambio);
        Global::Widget::v_main_stack->set_visible_child(*this); 
        v_lbl_monto_total->set_text(s_total);
        v_lbl_faltante->set_text(s_total);
        v_lbl_recibido->set_text("0"); 
    });

    Pago::da_pago(balance.cambio.load(), "Cambio Automatico", estatus, true);

    auto final_bill = Device::dv_bill.get_level_cash_actual(true, false);
    auto final_coin = Device::dv_coin.get_level_cash_actual(true, false);

    auto detalle_store = Gio::ListStore<MDetalleMovimiento>::create();

    Log log;
    crow::json::wvalue data;
    auto t_log = MLog::create(
        0,
        Global::User::id,
        "Cambio Automatico",
        concepto,
        0,
        balance.cambio.load(),
        0,
        not Global::Utility::is_ok ? estatus : "Cambio Realizada con Exito.",
        Glib::DateTime::create_now_local());

    t_log->m_id = log.insert_log(t_log);
    Global::Utility::is_ok = true;

    if (Global::Widget::Impresora::v_switch_impresion->get_active())
    {
        std::string command = "echo \"" + Global::System::imprime_ticket(t_log, faltante) + "\" | lp";
        std::system(command.c_str());
    }

    for (const auto& [denom, qty] : Pago::salidas_totales)
    {
        auto detalle = MDetalleMovimiento::create(0, t_log->m_id, "salida", denom, qty);
        detalle_store->append(detalle);
    }

    Pago::salidas_totales.clear();

    auto bd_detalle = std::make_unique<DetalleMovimiento>();
    bd_detalle->insertar_detalle_movimiento(t_log->m_id, detalle_store);

    data = Global::Utility::json_ticket(t_log);
    data["Cambio_faltante"] = Pago::faltante;

    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();

    async_gui.dispatch_to_gui([this]() { Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });

    balance.ingreso.store(0);
    balance.cambio.store(0);

    return crow::response(data);
}

crow::response Pago::inicia_cambio_manual(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Cambio_M);
    using namespace Global::EValidador;
    auto bodyParams = crow::json::load(req.body);
    Global::Utility::is_ok = true;
    Pago::faltante = 0;
    is_running.store(true);

    Device::dv_coin.inicia_dispositivo_v6();
    Device::dv_bill.inicia_dispositivo_v6();

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

    auto future1 = std::async(std::launch::async, [this](){ Device::dv_coin.poll(sigc::mem_fun(*this, &Pago::poll_cambio)); });
    auto future2 = std::async(std::launch::async, [this](){ Device::dv_bill.poll(sigc::mem_fun(*this, &Pago::poll_cambio)); });

    future1.wait();
    future2.wait();

    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();

    int cambio = balance.ingreso.load();
    crow::json::wvalue data;
    
    data["ingresado"] = cambio;
    data["pos_coin"] = Global::Utility::find_position(map_coin, cambio);
    data["pos_bill"] = Global::Utility::find_position(map_bill, cambio);
    balance.ingreso = 0;
    return crow::response(data);
}

crow::response Pago::termina_cambio_manual(const crow::request &req)
{
    estatus.clear();
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Cambio_M);
    auto bodyParams = crow::json::load(req.body);
    auto [cambio, bill_values, coin_values] = procesar_parametros_iniciales(bodyParams);

    auto snapshot_inicial_bill = Device::dv_bill.get_level_cash_actual(true, false);
    auto snapshot_inicial_coin = Device::dv_coin.get_level_cash_actual(true, false);


    validar_inventario_disponible(bill_values, coin_values);
    configurar_estado_pago(cambio);
    mostrar_interfaz_pago_manual(calcular_total_pago(bill_values, coin_values));

    auto t_log = registrar_pago_y_log(bill_values, coin_values, "Cambio Manual");

    auto final_bill = Device::dv_bill.get_level_cash_actual(true, false);
    auto final_coin = Device::dv_coin.get_level_cash_actual(true, false);

    auto diff_bill = DetalleMovimiento::calcular_diferencias_niveles(snapshot_inicial_bill, final_bill);
    auto diff_coin = DetalleMovimiento::calcular_diferencias_niveles(snapshot_inicial_coin, final_coin);

    auto detalle_store = Gio::ListStore<MDetalleMovimiento>::create();

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


    imprimir_ticket_si_corresponde(t_log);

    return finalizar_proceso_pago(t_log);
}

crow::response Pago::cancelar_cambio_manual(const crow::request &req)
{
    auto bodyParams = crow::json::load(req.body);
    int cambio = Global::EValidador::balance.cambio = bodyParams["value"].i();
    std::string concepto = bodyParams["concepto"].operator std::string();
    if (cambio <= 0)
        return crow::response("Nada que devolver");

    Pago::da_pago(cambio, "Cambio Manual", estatus);

    Log log;
    crow::json::wvalue data;
    auto t_log = MLog::create(
        0,
        Global::User::id,
        "Cambio Manual",
        concepto.empty() ? "Sin Concepto" : concepto,
        0,
        cambio,
        0,
        not Global::Utility::is_ok ? estatus : "Cambio Cancelado",
        Glib::DateTime::create_now_local());

    t_log->m_id = log.insert_log(t_log);
    Global::Utility::is_ok = true;

    if (Global::Widget::Impresora::v_switch_impresion->get_active())
    {
        std::string command = "echo \"" + Global::System::imprime_ticket(t_log, faltante) + "\" | lp";
        std::system(command.c_str());
    }

    data = Global::Utility::json_ticket(t_log);
    data["Cambio_faltante"] = Pago::faltante;

    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();

    return crow::response(data);
}
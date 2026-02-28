#include "controller/venta.hpp"

Venta::Venta(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : BVentaPago(cobject, refBuilder),
                                                                                      faltante(0)
{
    v_lbl_titulo->set_text("Venta");
    v_btn_timeout_retry->set_visible(false);

    async_gui.dispatcher.connect(sigc::mem_fun(async_gui, &Global::Async::on_dispatcher_emit));
    hub.on_credito().connect(sigc::mem_fun(*this, &Venta::on_event_credit));

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
    CROW_LOG_INFO << "WebSocket connected: " << conn.get_remote_ip();
    connection = &conn;

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Esperar a que el cliente esté listo para recibir mensajes

    crow::json::wvalue response;
    response["ingreso"] = ingreso;
    response["cambio"] = cambio;
    response["total"] = total;
    // response["terminado"] = !is_running;
    // response["status"] = is_running ? "En proceso" : "Proceso terminado";
    response["faltante"] = faltante;

    conn.send_text(response.dump());
}

void Venta::on_wb_socket_close(crow::websocket::connection &conn, const std::string &reason, uint16_t code)
{
    if (connection == &conn)
        connection = nullptr; // Limpiamos la referencia a la conexión cerrada
    CROW_LOG_WARNING << "WebSocket disconnected: " << conn.get_remote_ip() << " Reason: " << reason << " Code: " << code;
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
    //@@is_running.store(false);
    cancelado = true;
    async_gui.dispatch_to_gui([this]() { Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });
}

crow::response Venta::deten(const crow::request &req)
{
    on_btn_cancel_click();
    return crow::response(200, "Venta detenida");
}

void Venta::on_event_credit(const crow::json::rvalue &data, size_t credito)
{
    if (connection)
    {
        crow::json::wvalue response;
        response["ingreso"] = ingreso;
        response["cambio"] = cambio;
        response["total"] = total;
        response["terminado"] = !transaccion_terminada;
        response["status"] = transaccion_terminada ? "En proceso" : "Proceso terminado";
        response["faltante"] = faltante;

        connection->send_text(response.dump());
    }

    ingreso += credito;
    faltante = (total > ingreso) ? (total - ingreso) : 0;

    v_lbl_recibido->set_text(Glib::ustring::format(ingreso));
    v_lbl_faltante->set_text(Glib::ustring::format(faltante));
    v_lbl_cambio->set_text(Glib::ustring::format(faltante));

    if (ingreso >= total || cancelado)
    {
        hub.detiene_for_all();
        
        // NOTIFICACIÓN: Despertar al endpoint
        {
            std::lock_guard<std::mutex> lock(mtx_espera);
            transaccion_terminada = true;
        }
        cv_finalizado.notify_one(); 
    }
}

crow::response Venta::inicia(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Venta);
    Log log;

    auto bodyParams = crow::json::load(req.body);
    cancelado = false;
    estatus.clear();
    total = faltante = bodyParams["value"].i();
    cambio = ingreso = 0;
    concepto = bodyParams["concepto"].operator std::string();

    auto t_log = create_log(log);

    is_view_ingreso = bodyParams.has("is_view_ingreso") && bodyParams["is_view_ingreso"].b();
    is_view_ingreso ? v_lbl_titulo->set_text("Ingreso") : v_lbl_titulo->set_text("Venta");

    Conf conf;
    conf.habilita_recolector = true;
    conf.auto_acepta_credito = true;
    conf.habilita_salida_credito = true;

    hub.inicia_for_all(conf, {});
    hub.inicia_poll_for_all();

    async_gui.dispatch_to_gui([this]()
    { 
        Global::Widget::v_main_stack->set_visible_child(*this); 
        v_lbl_monto_total->set_text(Glib::ustring::format(total));
        v_lbl_faltante->set_text(Glib::ustring::format(total));
        v_lbl_cambio->set_text("0");
        v_lbl_recibido->set_text("0"); 
    });

    {
        std::unique_lock<std::mutex> lock(mtx_espera);
        cv_finalizado.wait(lock, [this] { return transaccion_terminada; });
    }
    
    CROW_LOG_INFO << "Transacción finalizada. Procesando resultado...";

    if (cancelado)
        t_log->m_estatus = "Operación cancelada";

    // if (cambio > 0)
        // Pago::da_pago(cambio, is_view_ingreso ? "Ingreso" : "Venta", estatus);
    if (Pago::faltante > 0)
        t_log->m_estatus  = "Cambio Incompleto, faltante: " + std::to_string(Pago::faltante);

    crow::json::wvalue data;
    

    data = Global::Utility::json_ticket(t_log);
    data["Cambio_faltante"] = Pago::faltante;

    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });

    return crow::response("Fin");
}


Glib::RefPtr<MLog> Venta::create_log(Log log)
{
    auto t_log = MLog::create
    (
        0,
        Global::User::id,
        is_view_ingreso ? "Ingreso" : "Venta",
        concepto.empty() ? "--" : concepto,
        0,
        0,
        total,
        "Creacion de evento",
        Glib::DateTime::create_now_local()
    );

    t_log->m_id = log.insert_log(t_log);
    return t_log;
}
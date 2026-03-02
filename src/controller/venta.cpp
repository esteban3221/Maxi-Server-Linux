#include "controller/venta.hpp"

Venta::Venta(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : BVentaPago(cobject, refBuilder),
                                                                                      faltante(0)
{
    v_lbl_titulo->set_text("Venta");
    v_btn_timeout_retry->set_visible(false);

    async_gui.dispatcher.connect(sigc::mem_fun(async_gui, &Global::Async::on_dispatcher_emit));
    hub.on_credito().connect(sigc::mem_fun(*this, &Venta::on_event_credit));
    hub.on_error().connect(sigc::mem_fun(*this, &Venta::on_error));

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
    cancelado = true;
    hub.detiene_for_all();
    async_gui.dispatch_to_gui([this]() { Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });
}

crow::response Venta::deten(const crow::request &req)
{
    on_btn_cancel_click();
    return crow::response(200, "Venta detenida");
}

void Venta::on_error(const std::string &error)
{
    t_log->m_estatus = error;
    log.update_log(t_log);
}


void Venta::on_event_credit(const crow::json::rvalue &data, size_t credito)
{
    t_log->m_ingreso += credito;
    t_log->m_cambio  = (t_log->m_ingreso > t_log->m_total) ? (t_log->m_ingreso - t_log->m_total) : 0;
    t_log->m_estatus = "Cobrando...";
    t_log->m_fecha = Glib::DateTime::create_now_local();
    faltante = (t_log->m_ingreso > t_log->m_total) ? 0 : (t_log->m_total - t_log->m_ingreso);

    v_lbl_recibido->set_text(Glib::ustring::format(t_log->m_ingreso));
    v_lbl_faltante->set_text(Glib::ustring::format(faltante));
    v_lbl_cambio->set_text(Glib::ustring::format(t_log->m_cambio));

    log.update_log(t_log);

    if (t_log->m_ingreso >= t_log->m_total || cancelado)
    {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        hub.detiene_poll_for_all();
        
        // NOTIFICACIÓN: Despertar al endpoint
        {
            std::lock_guard<std::mutex> lock(mtx_espera);
            transaccion_terminada = true;
        }
        cv_finalizado.notify_one(); 
    }

    if (connection)
    {
        crow::json::wvalue response;
        response["ingreso"] = t_log->m_ingreso;
        response["cambio"] = t_log->m_cambio;
        response["total"] = t_log->m_total;
        response["terminado"] = !transaccion_terminada;
        response["status"] = transaccion_terminada ? "En proceso" : "Proceso terminado";
        response["faltante"] = faltante;

        connection->send_text(response.dump());
    }
}

crow::response Venta::inicia(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Venta);

    reset_log(crow::json::load(req.body));

    Conf conf;
    conf.habilita_recolector = true;
    conf.auto_acepta_credito = true;
    conf.habilita_salida_credito = true;

    hub.inicia_for_all(conf, {});
    hub.inicia_poll_for_all();

    async_gui.dispatch_to_gui([this]()
    { 
        Global::Widget::v_main_stack->set_visible_child(*this); 
        v_lbl_monto_total->set_text(Glib::ustring::format(t_log->m_total));
        v_lbl_faltante->set_text(Glib::ustring::format(t_log->m_total));
        v_lbl_cambio->set_text("0");
        v_lbl_recibido->set_text("0"); 
    });

    {
        std::unique_lock<std::mutex> lock(mtx_espera);
        cv_finalizado.wait(lock, [this] { return transaccion_terminada; });
    }

    if (cancelado)
        t_log->m_estatus = "Operación cancelada";
    else
        t_log->m_estatus = "Exito.";
    if (t_log->m_cambio > 0 && !cancelado)
        hub.inicia_pago(t_log->m_cambio);
    
    CROW_LOG_INFO << "Transacción finalizada. Procesando resultado...";

    log.update_log(t_log);    
    hub.detiene_for_all();

    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });

    return crow::response(Global::Utility::json_ticket(t_log));
}


void Venta::reset_log(const crow::json::rvalue &param)
{
    transaccion_terminada = cancelado = false;

    is_view_ingreso = param.has("is_view_ingreso") && param["is_view_ingreso"].b();
    is_view_ingreso ? v_lbl_titulo->set_text("Ingreso") : v_lbl_titulo->set_text("Venta");

    t_log = MLog::create
    (
        0,
        Global::User::id,
        is_view_ingreso ? "Ingreso" : "Venta",
        param["concepto"].operator std::string(),
        0,
        0,
        param["value"].i(),
        "Creacion de evento",
        Glib::DateTime::create_now_local()
    );

    t_log->m_id = log.insert_log(t_log);
}
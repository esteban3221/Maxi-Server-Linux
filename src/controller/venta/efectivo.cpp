#include "controller/venta/efectivo.hpp"

Efectivo::Efectivo(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder, crow::SimpleApp& app) : BVentaPago(cobject, refBuilder)
{
    v_lbl_titulo->set_text("Venta");
    v_btn_timeout_retry->set_visible(false);

    async_gui.dispatcher.connect(sigc::mem_fun(async_gui, &Global::Async::on_dispatcher_emit));

    CROW_ROUTE(app, "/accion/detiene_venta").methods("GET"_method)(sigc::mem_fun(*this, &Efectivo::deten));

    CROW_WEBSOCKET_ROUTE(app, "/ws/venta")
        .onopen(sigc::mem_fun(*this, &Efectivo::on_wb_socket_open))
        .onclose(sigc::mem_fun(*this, &Efectivo::on_wb_socket_close))
        .onmessage(sigc::mem_fun(*this, &Efectivo::on_wb_socket_message));
}

Efectivo::~Efectivo()
{
}

void Efectivo::on_wb_socket_open(crow::websocket::connection &conn)
{
    CROW_LOG_INFO << "WebSocket connected: " << conn.get_remote_ip();
    connection = &conn;
}

void Efectivo::on_wb_socket_close(crow::websocket::connection &conn, const std::string &reason, uint16_t code)
{
    if (connection == &conn)
        connection = nullptr; // Limpiamos la referencia a la conexión cerrada
    CROW_LOG_WARNING << "WebSocket disconnected: " << conn.get_remote_ip() << " Reason: " << reason << " Code: " << code;
}

void Efectivo::on_wb_socket_message(crow::websocket::connection &conn, const std::string &data, bool is_binary)
{
    auto json_data = crow::json::load(data);

    if (json_data["action"] == "detener")
    {
        on_btn_cancel_click();
        conn.send_text(R"({"status":"detenido"})");
    }
}

void Efectivo::on_btn_retry_click()
{
    CROW_LOG_INFO << "Click otra vez desde Venta\n";
}

void Efectivo::on_btn_cancel_click()
{
    std::lock_guard<std::mutex> lock(mtx_espera);
    if (transaccion_terminada) return;

    transaccion_terminada = cancelado = true;
    cv_finalizado.notify_one(); 
}

crow::response Efectivo::deten(const crow::request &req)
{
    on_btn_cancel_click();
    return crow::response(200, "Venta detenida");
}

void Efectivo::on_error(const std::string &device, const std::string &error)
{
    t_log->m_estatus = error;
    log.update_log(t_log);
}

void Efectivo::on_event_credit(const std::string &device_id, const std::string &type, const crow::json::rvalue &data, size_t credito)
{
    t_log->m_ingreso += credito;
    t_log->m_cambio  = (t_log->m_ingreso > t_log->m_total) ? (t_log->m_ingreso - t_log->m_total) : 0;
    t_log->m_estatus = "Cobrando...";
    t_log->m_fecha = Glib::DateTime::create_now_local();
    auto faltante = (t_log->m_ingreso > t_log->m_total) ? 0 : (t_log->m_total - t_log->m_ingreso);

    v_lbl_recibido->set_text(Glib::ustring::format(t_log->m_ingreso));
    v_lbl_faltante->set_text(Glib::ustring::format(faltante));
    v_lbl_cambio->set_text(Glib::ustring::format(t_log->m_cambio));

    log.update_log(t_log);

		if (connection)
    {
        crow::json::wvalue response;
        response["ingreso"] = t_log->m_ingreso;
        response["cambio"] = t_log->m_cambio;
        response["total"] = t_log->m_total;
        response["terminado"] = transaccion_terminada;
        response["status"] = !transaccion_terminada ? "En proceso" : "Proceso terminado";
        response["faltante"] = faltante;

        connection->send_text(response.dump());
    }

    if (t_log->m_ingreso >= t_log->m_total || cancelado)
    {
        t_log->m_estatus = "Finalizado";
        log.update_log(t_log);

        hub.command_for_all(HttpMethod::POST, "DisableAcceptor");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        hub.detiene_poll_for_all(t_log->m_id);
        {
            std::lock_guard<std::mutex> lock(mtx_espera);
            transaccion_terminada = true;
        }
        cv_finalizado.notify_one(); 
    }
}

crow::response Efectivo::inicia(Glib::RefPtr<MLog> t_log, bool is_view_ingreso)
{
    // Sesion::valida_autorizacion(req, Global::User::Rol::Venta); se tiene que mover al padre de esta vista, para que el proceso de venta pueda ser iniciado desde otras vistas como ingreso o corte.
    transaccion_terminada = cancelado = false;
    this->t_log = t_log;
    hub.on_credito().connect(sigc::mem_fun(*this, &Efectivo::on_event_credit));
    hub.on_error().connect(sigc::mem_fun(*this, &Efectivo::on_error));

    is_view_ingreso ? v_lbl_titulo->set_text("Ingreso") : v_lbl_titulo->set_text("Venta");

    Conf conf;
    conf.habilita_recolector = true;
    conf.auto_acepta_credito = true;
    conf.habilita_salida_credito = true;

    hub.inicia_for_all(conf);
    hub.inicia_poll_for_all();

    async_gui.dispatch_to_gui([this]()
    { 
        Global::Widget::v_main_stack->set_visible_child(*this); 
        v_lbl_monto_total->set_text(Glib::ustring::format(this->t_log->m_total));
        v_lbl_faltante->set_text(Glib::ustring::format(this->t_log->m_total));
        v_lbl_cambio->set_text("0");
        v_lbl_recibido->set_text("0"); 
    });

    {
        std::unique_lock<std::mutex> lock(mtx_espera);
        cv_finalizado.wait(lock, [this] { return transaccion_terminada; });
    }

    if (cancelado)
    {
        t_log->m_estatus = "Operación cancelada";
        if (t_log->m_ingreso > 0)
            hub.inicia_pago(t_log->m_id, t_log->m_ingreso);
        t_log->m_cambio = t_log->m_ingreso;
    }
    else
        t_log->m_estatus = "Completado";
    if (t_log->m_cambio > 0 && !cancelado)
        hub.inicia_pago(t_log->m_id, t_log->m_cambio);

    log.update_log(t_log);    
    hub.detiene_for_all();
    hub.on_credito().clear();
    hub.on_error().clear();

    // hay que implementar una funcion que determine si hay que volver ala vista de seleccion de pago en caso de que el pago sea diferente a efectivo,
    // o si se tiene que volver a la vista principal en caso de que el pago se haya completado con efectivo o se haya cancelado la operacion.
    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });

    return crow::response(Log::json_ticket(t_log));
}
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
}

void Pago::on_btn_cancel_click()
{
}

crow::response Pago::inicia(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Pago);

    reset_log(crow::json::load(req.body), "Pago");
    t_log->m_estatus = "Exito.";

    Conf conf;
    conf.habilita_recolector = false;
    conf.auto_acepta_credito = false;
    conf.habilita_salida_credito = true;

    hub.inicia_for_all(conf);
    hub.inicia_pago(t_log->m_id, t_log->m_cambio);

    log.update_log(t_log);
    hub.detiene_for_all();

    hub.on_credito().clear();
    hub.on_error().clear();

    async_gui.dispatch_to_gui([this]() { Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });

    return crow::response(Log::json_ticket(t_log));
}

crow::response Pago::inicia_manual(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Pago);
    auto json = crow::json::load(req.body);
    std::map<std::string, std::string> map_val;

    reset_log(json, "Pago Manual");
    t_log->m_estatus = "Exito.";

    for (auto &&k : json["pago_manual"].keys())
        map_val[k] = crow::json::wvalue(json["pago_manual"][k]).dump();

    Conf conf;
    conf.habilita_recolector = false;
    conf.auto_acepta_credito = false;
    conf.habilita_salida_credito = true;

    hub.inicia_for_all(conf);
    hub.inicia_pago(t_log->m_id, map_val);

    log.update_log(t_log);
    hub.detiene_for_all();
    hub.on_credito().clear();
    hub.on_error().clear();

    async_gui.dispatch_to_gui([this]() { Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });

    return crow::response(Log::json_ticket(t_log));
}

crow::response Pago::inicia_cambio(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Cambio_A);
    reset_log(crow::json::load(req.body), "Cambio");
    t_log->m_estatus = "Exito.";

    Conf conf;
    conf.habilita_recolector = false;
    conf.auto_acepta_credito = false;
    conf.habilita_salida_credito = true;

    hub.inicia_for_all(conf);
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

    hub.inicia_pago(t_log->m_ingreso, true);
    log.update_log(t_log);
    hub.detiene_for_all();
    hub.on_credito().clear();
    hub.on_error().clear();

    async_gui.dispatch_to_gui([this]() { Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });

    return crow::response(Log::json_ticket(t_log));
}

crow::response Pago::inicia_cambio_manual(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Cambio_M);
    reset_log(crow::json::load(req.body), "Cambio Manual");
    t_log->m_estatus = "Exito.";

    Conf conf;
    conf.habilita_recolector = false;
    conf.auto_acepta_credito = false;
    conf.habilita_salida_credito = true;

    hub.inicia_for_all(conf);
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

    return crow::response(200);
}

crow::response Pago::termina_cambio_manual(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Cambio_M);

    auto json = crow::json::load(req.body);
    std::map<std::string, std::string> map_val;

    for (auto &&k : json["pago_manual"].keys())
        map_val[k] = crow::json::wvalue(json["pago_manual"][k]).dump();

    hub.inicia_pago(t_log->m_id, map_val);
    log.update_log(t_log);
    hub.detiene_for_all();

    async_gui.dispatch_to_gui([this]() { Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });
    hub.on_credito().clear();
    hub.on_error().clear();

    return crow::response(Log::json_ticket(t_log));
}

crow::response Pago::cancelar_cambio_manual(const crow::request &req)
{
    auto bodyParams = crow::json::load(req.body);

    hub.inicia_pago(t_log->m_id, t_log->m_ingreso);
    log.update_log(t_log);
    hub.detiene_for_all();

    async_gui.dispatch_to_gui([this]() { Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });
    hub.on_credito().clear();
    hub.on_error().clear();

    return crow::response(Log::json_ticket(t_log));
}

void Pago::on_error(const std::string &device, const std::string &error)
{
    t_log->m_estatus = error;
    log.update_log(t_log);
}

void Pago::on_credit(const std::string &, const std::string &, const crow::json::rvalue &data, size_t credito)
{
    t_log->m_total = t_log->m_ingreso += credito;
    log.update_log(t_log);

    std::this_thread::sleep_for(std::chrono::seconds(3));
    hub.detiene_poll_for_all(t_log->m_id);

    // NOTIFICACIÓN: Despertar al endpoint
    {
        std::lock_guard<std::mutex> lock(mtx_espera);
        transaccion_terminada = true;
    }
    cv_finalizado.notify_one();
}

void Pago::reset_log(const crow::json::rvalue &param, const std::string &type)
{
    hub.on_credito().connect(sigc::mem_fun(*this, &Pago::on_credit));
    hub.on_error().connect(sigc::mem_fun(*this, &Pago::on_error));

    t_log = MLog::create
    (
        0,
        Global::User::id,
        type,
        param["concepto"].operator std::string(),
        0,
        param.has("value") ? param["value"].i() : param["total"].i(),
        0,
        "Creacion de evento",
        Glib::DateTime::create_now_local()
    );

    t_log->m_id = log.insert_log(t_log);
}
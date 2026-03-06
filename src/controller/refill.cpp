#include "controller/refill.hpp"

Refill::Refill(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VRefill(cobject, refBuilder)
{
    async_gui.dispatcher.connect(sigc::mem_fun(async_gui, &Global::Async::on_dispatcher_emit));

    init_data(v_tree_reciclador_monedas, "Level_Coin");
    init_data(v_tree_reciclador_billetes, "Level_Bill");

    this->signal_map().connect(sigc::mem_fun(*this, &Refill::on_show_map));
    v_btn_deten->signal_clicked().connect(sigc::mem_fun(*this, &Refill::deten));

    CROW_ROUTE(RestApp::app, "/accion/inicia_refill").methods("POST"_method)(sigc::mem_fun(*this, &Refill::inicia));
    CROW_ROUTE(RestApp::app, "/accion/deten_refill").methods("GET"_method)(sigc::mem_fun(*this, &Refill::deten_remoto));
    CROW_ROUTE(RestApp::app, "/validadores/get_dashboard").methods("GET"_method)(sigc::mem_fun(*this, &Refill::get_dashboard));
    CROW_ROUTE(RestApp::app, "/validadores/update_imovilidad").methods("POST"_method)(sigc::mem_fun(*this, &Refill::update_imovilidad));

    CROW_ROUTE(RestApp::app, "/validador/transpaso").methods("POST"_method)(sigc::mem_fun(*this, &Refill::transpaso));
    CROW_ROUTE(RestApp::app, "/validador/retirada").methods("POST"_method)(sigc::mem_fun(*this, &Refill::retirada));

    CROW_WEBSOCKET_ROUTE(RestApp::app, "/ws/refill")
        .onopen(sigc::mem_fun(*this, &Refill::on_wb_socket_open))
        .onclose(sigc::mem_fun(*this, &Refill::on_wb_socket_close))
        .onmessage(sigc::mem_fun(*this, &Refill::on_wb_socket_message));
}

Refill::~Refill()
{
}

void Refill::init_data(Gtk::ColumnView *vcolumn, const std::string &tabla)
{
    auto level = std::make_unique<LevelCash>(tabla);
    auto m_list = level->get_level_cash();
    auto selection_model = Gtk::SingleSelection::create(m_list);

    vcolumn->set_model(selection_model);
    vcolumn->add_css_class("data-table");

    {
        auto factory = Gtk::SignalListItemFactory::create();
        factory->signal_setup().connect(sigc::mem_fun(*this, &Refill::on_setup_label));
        factory->signal_bind().connect(sigc::mem_fun(*this, &Refill::on_bind_deno));
        auto column = Gtk::ColumnViewColumn::create("Denominacion", factory);
        vcolumn->append_column(column);
    }

    if (tabla == "Level_Bill")
    {
        auto factory = Gtk::SignalListItemFactory::create();
        factory->signal_setup().connect(sigc::mem_fun(*this, &Refill::on_setup_label));
        factory->signal_bind().connect(sigc::mem_fun(*this, &Refill::on_bind_alm));
        auto column = Gtk::ColumnViewColumn::create("Casette", factory);
        column->set_expand(true);
        vcolumn->append_column(column);
    }

    {
        auto factory = Gtk::SignalListItemFactory::create();
        factory->signal_setup().connect(sigc::mem_fun(*this, &Refill::on_setup_label));
        factory->signal_bind().connect(sigc::mem_fun(*this, &Refill::on_bind_recy));
        auto column = Gtk::ColumnViewColumn::create("Recylador", factory);
        column->set_expand(true);
        vcolumn->append_column(column);
    }

    {
        auto factory = Gtk::SignalListItemFactory::create();
        factory->signal_setup().connect(sigc::mem_fun(*this, &Refill::on_setup_label));
        factory->signal_bind().connect(sigc::mem_fun(*this, &Refill::on_bind_inmo));
        auto column = Gtk::ColumnViewColumn::create("Inmovilidad", factory);
        column->set_expand(true);
        vcolumn->append_column(column);
    }

    {
        auto factory = Gtk::SignalListItemFactory::create();
        factory->signal_setup().connect(sigc::mem_fun(*this, &Refill::on_setup_label));
        factory->signal_bind().connect(sigc::mem_fun(*this, &Refill::on_bind_ingreso));
        auto column = Gtk::ColumnViewColumn::create("Ingreso", factory);
        vcolumn->append_column(column);
    }
}

void Refill::on_show_map()
{
    // si algun dia existe la necesidad de obtener los niveles especificos
    // en caso de haber mas de 2 validadores se realizara el cambio por 
    // ahora se generalizara por tipo

    Glib::signal_idle().connect_once([this]()
    {
        auto map = hub.command_for_all(HttpMethod::GET, "GetAllLevels");

        for (auto const& [llave, valor] : map)
        {
            if(valor.status_code != cpr::status::HTTP_OK)
            {
                CROW_LOG_ERROR << "No se pudieron cargar los niveles para el dispositivo " << llave << " - " << valor.error.message;
                continue;
            }

            auto es_moneda = (valor.header.at("X-Device-Type") == "COIN");
            auto selection = es_moneda ? v_tree_reciclador_monedas->get_model() : v_tree_reciclador_billetes->get_model();
            auto single = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection);
            auto list_store = std::dynamic_pointer_cast<Gio::ListStore<MLevelCash>>(single->get_model());

            list_store->remove_all();

            auto json = crow::json::load(valor.text);
            auto level = std::make_unique<LevelCash>(es_moneda ? "Level_Coin" : "Level_Bill")->get_level_cash();

            for (size_t i = 0; i < json.size(); i++)
            {
                list_store->append(MLevelCash::create
                    (
                        json[i]["value"].i() / 100,
                        json[i]["storedInPayout"].i(),
                        json[i]["storedInCashbox"].i(),
                        level->get_item(i)->m_nivel_inmo_min,
                        level->get_item(i)->m_nivel_inmo,
                        level->get_item(i)->m_nivel_inmo_max,
                        0
                    ));
            }
        }
    });
}

void Refill::on_credit(const std::string &device_id, const std::string &type, const crow::json::rvalue &data, size_t credito)
{
    auto selection = (type == "COIN") ? v_tree_reciclador_monedas->get_model() : v_tree_reciclador_billetes->get_model();
    auto single = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection);
    auto list_store = std::dynamic_pointer_cast<Gio::ListStore<MLevelCash>>(single->get_model());

    // para este momento ya esta conectado y funcionando asi que confio en que siempre devuelvan datos :)
    auto response = hub.command_by_device_id(HttpMethod::GET, device_id, "GetAllLevels");
    auto json = crow::json::load(response.text);
    
    crow::json::wvalue json_ws;
    json_ws[type] = crow::json::wvalue::list();

    for (size_t i = 0; i < list_store->get_n_items(); i++)
    {
        auto item = list_store->get_item(i);
        for (auto &&j : json)
        {
            if (item->m_denominacion == j["value"].i())
            {
                async_gui.dispatch_to_gui([this, &item, json]()
                {
                    item->m_ingreso = json["storedInPayout"].i() - item->m_cant_recy;
                });
                break;
            }
        }

        json_ws[type][i]["Denominacion"] = item->m_denominacion;
        json_ws[type][i]["Almacenado"] = item->m_cant_alm;
        json_ws[type][i]["Recyclador"] = item->m_cant_recy;
        json_ws[type][i]["Inmovilidad_Min"] = item->m_nivel_inmo_min;
        json_ws[type][i]["Inmovilidad"] = item->m_nivel_inmo;
        json_ws[type][i]["Inmovilidad_Max"] = item->m_nivel_inmo_max;
        json_ws[type][i]["Ingreso"] = item->m_ingreso;
    }

    //conn.send_text(json.dump());
}

void Refill::on_error(const std::string &error)
{
    t_log->m_estatus = error;
    log.update_log(t_log);
}

crow::response Refill::deten_remoto(const crow::request &req)
{
    deten();
    return crow::response(200);
}

crow::response Refill::inicia(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Carga);
    reset_log(crow::json::load(req.body));

    Conf conf;
    conf.habilita_recolector = true;
    conf.auto_acepta_credito = false;
    conf.habilita_salida_credito = true;

    hub.inicia_for_all(conf);
    hub.inicia_poll_for_all();

    async_gui.dispatch_to_gui([this]()
    { 
        Global::Widget::v_main_stack->set_visible_child(*this); 
    });

    {
        std::unique_lock<std::mutex> lock(mtx_espera);
        cv_finalizado.wait(lock, [this] { return transaccion_terminada; });
    }

    hub.detiene_for_all();
    hub.on_credito().clear();
    hub.on_error().clear();

    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });

    return crow::response(Log::json_ticket(t_log));
}

crow::response Refill::get_dashboard(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Consulta_Efectivo);
    


    return crow::response();
}

crow::response Refill::update_imovilidad(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Configuracion);
    auto data = crow::json::load(req.body);

    auto denominacion = data["denominacion"].i();
    auto nivel_inmo_min = data["nivel_inmo_min"].i();
    auto nivel_inmo = data["nivel_inmo"].i();
    auto nivel_inmo_max = data["nivel_inmo_max"].i();

    auto bd = denominacion > 10 ? std::make_unique<LevelCash>("Level_Bill") : std::make_unique<LevelCash>("Level_Coin");
    bd->update_nivel_inmo(denominacion,nivel_inmo_min, nivel_inmo, nivel_inmo_max);
    return crow::response();
}

crow::response Refill::transpaso(const crow::request &req)
{


    // return crow::response(status.first, data);
    return {};
}

size_t Refill::saca_cassette()
{
    size_t total_bill = 0;
    


    return total_bill;
}

crow::response Refill::retirada(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Retirada);
    async_gui.dispatch_to_gui([this]()
    { Global::Widget::v_main_stack->set_visible_child(*this); });

    Log log;
    auto t_log = MLog::create(0, Global::User::id, "Retirada de Casette", "", 0, 0, saca_cassette(), "Completado", Glib::DateTime::create_now_local());
    t_log->m_id = log.insert_log(t_log);
    auto data = Log::json_ticket(t_log);

    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });
    
    return crow::response(200, data);
}

void Refill::deten()
{
    {
        std::lock_guard<std::mutex> lock(mtx_espera);
        transaccion_terminada = true;
    }

    cv_finalizado.notify_one(); 
}

void Refill::reset_log(const crow::json::rvalue &param)
{
    transaccion_terminada = cancelado = false;
    hub.on_credito().connect(sigc::mem_fun(*this, &Refill::on_credit));
    hub.on_error().connect(sigc::mem_fun(*this, &Refill::on_error));

    t_log = MLog::create
    (
        0,
        Global::User::id,
        "Refill",
        param["concepto"].operator std::string(),
        0,
        0,
        0,
        "Exito.",
        Glib::DateTime::create_now_local()
    );

    t_log->m_id = log.insert_log(t_log);
}

void Refill::on_wb_socket_open(crow::websocket::connection &conn)
{
    CROW_LOG_INFO << "WebSocket connection opened: " << conn.get_remote_ip();
}

void Refill::on_wb_socket_close(crow::websocket::connection &conn, const std::string &reason, uint16_t code)
{
    CROW_LOG_INFO << "WebSocket connection closed: " << conn.get_remote_ip()
              << " Reason: " << reason
              << " Code: " << code;
}

void Refill::on_wb_socket_message(crow::websocket::connection &conn, const std::string &data, bool is_binary)
{
    auto json_data = crow::json::load(data);
    if (json_data.has("action") && json_data["action"] == "detener")
    {
        deten();
        conn.send_text(R"({"status":"detenido"})");
        return;
    }
}
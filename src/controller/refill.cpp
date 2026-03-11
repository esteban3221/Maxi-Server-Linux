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
        size_t total = 0;

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
                    json[i]["storedInCashbox"].i(),
                    json[i]["storedInPayout"].i(),
                    level->get_item(i)->m_nivel_inmo_min,
                    level->get_item(i)->m_nivel_inmo,
                    level->get_item(i)->m_nivel_inmo_max,
                    0
                ));
            }

            total += calcula_total(valor.header.at("X-Device-Type"), list_store);
        }

        v_lbl_total->set_text(Glib::ustring::format(total));
    });
}

size_t Refill::calcula_total(const std::string &type, const std::shared_ptr<Gio::ListStore<MLevelCash>> &list_store)
{
    size_t total = 0;
    size_t parcial = 0;

    auto label_total = type == "COIN" ? v_lbl_total_monedas : v_lbl_total_billetes;
    auto label_parcial = type == "COIN" ? v_lbl_total_parcial_monedas : v_lbl_total_parcial_billetes;

    for (size_t i = 0; i < list_store->get_n_items(); i++)
    {
        auto m_list = list_store->get_typed_object<const MLevelCash>(i);

        total += type == "COIN" ? 0 : m_list->m_cant_alm * m_list->m_denominacion;
        total += m_list->m_cant_recy * m_list->m_denominacion;

        parcial += m_list->m_ingreso * m_list->m_denominacion;
    }
    
    async_gui.dispatch_to_gui([this, total, parcial, label_total, label_parcial]()
    {
        label_total->set_text(Glib::ustring::format(total));
        label_parcial->set_text(Glib::ustring::format(parcial));
    });
    
    return total;
}

void Refill::on_credit(const std::string &device_id, const std::string &type, const crow::json::rvalue &data, size_t credito)
{
    t_log->m_ingreso += credito;
    log.update_log(t_log);

    auto selection = (type == "COIN") ? v_tree_reciclador_monedas->get_model() : v_tree_reciclador_billetes->get_model();
    auto single = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection);
    auto list_store = std::dynamic_pointer_cast<Gio::ListStore<MLevelCash>>(single->get_model());
    size_t total = 0;

    for (size_t i = 0; i < list_store->get_n_items(); i++)
    {
        auto item = list_store->get_item(i);
        
        if (item->m_denominacion == credito)
        {
            if(item->m_cant_recy <= item->m_nivel_inmo)
            {
                if (type == "BILL")
                    hub.command_by_device_id(HttpMethod::POST, device_id, "AcceptFromEscrow", "", true);
                
                item->m_ingreso++;
                item->m_cant_recy++;
                async_gui.dispatch_to_gui([this, item, i, list_store, single, &total, type]()
                {
                    list_store->remove(i);
                    list_store->insert(i, item);
                    single->select_item(i, true);
                    total += calcula_total(type, list_store);
                });
            
                envia_mensaje_wb(type, item);
            }
            else
                hub.command_by_device_id(HttpMethod::POST, device_id, "ReturnFromEscrow", "", true);
        }
    }
    v_lbl_total->set_text(Glib::ustring::format(total));
}

void Refill::envia_mensaje_wb(const std::string &device, const Glib::RefPtr<MLevelCash> &item)
{
    // Por el momento se generaliza por tipo, 
    // pero algun dia sera necesario por device_id, solo se cambia la variable de paso
    if(not conn_)
        return;
        
    crow::json::wvalue json_ws;
    json_ws[device] = crow::json::wvalue::list();

    json_ws[device][0]["Denominacion"] = item->m_denominacion;
    json_ws[device][0]["Almacenado"] = item->m_cant_alm;
    json_ws[device][0]["Recyclador"] = item->m_cant_recy;
    json_ws[device][0]["Inmovilidad_Min"] = item->m_nivel_inmo_min;
    json_ws[device][0]["Inmovilidad"] = item->m_nivel_inmo;
    json_ws[device][0]["Inmovilidad_Max"] = item->m_nivel_inmo_max;
    json_ws[device][0]["Ingreso"] = item->m_ingreso;
    
    conn_->send_text(json_ws.dump());
}

void Refill::on_error(const std::string &device, const std::string &error)
{
if (error == "CASHBOX_REMOVED")
    {
        hub.detiene_poll_for_all(-1); 

        auto response_level = hub.command_by_device_id(HttpMethod::GET, device, "GetAllLevels");
        auto json_level = crow::json::load(response_level.text);

        cashbox_level = 0;
        for (auto &&i : json_level)
            cashbox_level += (i["value"].i() / 100) * i["storedInCashbox"].i();
        hub.command_by_device_id(HttpMethod::POST, device, "ClearCashboxLevels");

        {
            std::lock_guard<std::mutex> lock(mtx_espera);
            transaccion_terminada = true;
        }

        cv_finalizado.notify_one(); 
    }
    else
    {
        t_log->m_estatus = error;
        log.update_log(t_log);
    }
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

    crow::json::wvalue json_final;
    auto &hub = CashHub::instance();
    auto map = hub.obten_ultimo_snapshot_level();

    for (auto const& [device_id, valor] : map)
    {
        auto niveles_hw = valor["levels"];
        auto tipo_db = (valor["type"].s() == "COIN" ? "Level_Coin" : "Level_Bill");
        auto config_db = std::make_unique<LevelCash>(tipo_db)->get_level_cash();

        crow::json::wvalue lista_hibrida = crow::json::wvalue::list();

        for (size_t i = 0; i < niveles_hw.size(); i++)
        {
            crow::json::wvalue item = niveles_hw[i];

            if (i < config_db->get_n_items()) 
            {
                auto db_item = config_db->get_item(i);
                item["Inmovilidad_Min"] = db_item->m_nivel_inmo_min;
                item["Inmovilidad"] = db_item->m_nivel_inmo;
                item["Inmovilidad_Max"] = db_item->m_nivel_inmo_max;
                // item["Ingreso"] = db_item->m_ingreso; 
            }
            lista_hibrida[i] = std::move(item);
        }
        json_final[device_id]["levels"] = std::move(lista_hibrida);
        json_final[device_id]["type"] = valor["type"].s();
    }
    return crow::response(json_final);
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

crow::response Refill::retirada(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Retirada);
    auto conn = hub.on_error().connect(sigc::mem_fun(*this, &Refill::on_error));
    transaccion_terminada = false;
    cashbox_level = 0;

    hub.inicia_for_all({});
    hub.inicia_poll_for_all();

    async_gui.dispatch_to_gui([this]()
    { 
        Global::Widget::v_main_stack->set_visible_child(*this); 
    });

    {
        std::unique_lock<std::mutex> lock(mtx_espera);
        if(!cv_finalizado.wait_for(lock, std::chrono::minutes(2), [this] { return transaccion_terminada; })) 
        {
             conn.disconnect();
             return crow::response(408, "Timeout: No se retiró el cassette");
        }
    }

    auto t_log = MLog::create(0, Global::User::id, "Retirada de Casette", "", 0, 0, cashbox_level, "Completado", Glib::DateTime::create_now_local());
    t_log->m_id = log.insert_log(t_log);

    hub.detiene_for_all();
    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });
    conn.disconnect();
    
    return crow::response(200, Log::json_ticket(t_log));
}

void Refill::deten()
{
    hub.detiene_poll_for_all(t_log->m_id);

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
        "",
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
    conn_ = &conn;
}

void Refill::on_wb_socket_close(crow::websocket::connection &conn, const std::string &reason, uint16_t code)
{
    CROW_LOG_INFO << "WebSocket connection closed: " << conn.get_remote_ip()
              << " Reason: " << reason
              << " Code: " << code;
    if(conn_ == &conn)
        conn_ = nullptr;
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
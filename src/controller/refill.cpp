#include "controller/refill.hpp"

Refill::Refill(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VRefill(cobject, refBuilder),
                                                                                        total(0),
                                                                                        total_bill(0),
                                                                                        total_coin(0),
                                                                                        parcial_bill(0),
                                                                                        parcial_coin(0)
{
    async_gui.dispatcher.connect(sigc::mem_fun(async_gui, &Global::Async::on_dispatcher_emit));

    init_data(Global::Widget::Refill::v_tree_reciclador_monedas, "Level_Coin");
    init_data(Global::Widget::Refill::v_tree_reciclador_billetes, "Level_Bill");

    this->signal_map().connect(sigc::mem_fun(*this, &Refill::on_show_map));
    v_btn_deten->signal_clicked().connect(sigc::mem_fun(*this, &Refill::deten));

    CROW_ROUTE(RestApp::app, "/accion/inicia_refill").methods("POST"_method)(sigc::mem_fun(*this, &Refill::inicia));
    CROW_ROUTE(RestApp::app, "/validadores/get_dashboard").methods("GET"_method)(sigc::mem_fun(*this, &Refill::get_dashboard));
    CROW_ROUTE(RestApp::app, "/validadores/update_imovilidad").methods("POST"_method)(sigc::mem_fun(*this, &Refill::update_imovilidad));

    CROW_ROUTE(RestApp::app, "/validador/transpaso").methods("POST"_method)(sigc::mem_fun(*this, &Refill::transpaso));
    CROW_ROUTE(RestApp::app, "/validador/retirada").methods("POST"_method)(sigc::mem_fun(*this, &Refill::retirada));
}

Refill::~Refill()
{
}

void Refill::calcula_total(const std::shared_ptr<Gtk::SingleSelection> &select_bill, const std::shared_ptr<Gtk::SingleSelection> &select_coin)
{
    total_bill = 0;
    total_coin = 0;
    parcial_bill = 0;
    parcial_coin = 0;

    for (size_t i = 0; i < select_bill->get_n_items(); i++)
    {
        auto m_list = select_bill->get_typed_object<const MLevelCash>(i);
        total_bill += m_list->m_cant_alm * m_list->m_denominacion;
        total_bill += m_list->m_cant_recy * m_list->m_denominacion;

        parcial_bill += m_list->m_ingreso * m_list->m_denominacion;
    }

    v_lbl_total_billetes->set_text(std::to_string(total_bill));
    v_lbl_total_parcial_billetes->set_text(std::to_string(parcial_bill));

    for (size_t i = 0; i < select_coin->get_n_items(); i++)
    {
        auto m_list = select_coin->get_typed_object<const MLevelCash>(i);
        total_coin += m_list->m_cant_alm * m_list->m_denominacion;
        total_coin += m_list->m_cant_recy * m_list->m_denominacion;

        parcial_coin += m_list->m_ingreso * m_list->m_denominacion;
    }

    v_lbl_total_parcial_monedas->set_text(std::to_string(parcial_coin));
    v_lbl_total_monedas->set_text(std::to_string(total_coin));

    v_lbl_total->set_text(std::to_string(total_bill + total_coin));
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
    Glib::signal_idle().connect_once([this]()
                                     {
    try
    {

        auto level_coin = Device::dv_coin.get_level_cash_actual(true);
        auto level_bill = Device::dv_bill.get_level_cash_actual(true);

        auto selection_bill_model = Global::Widget::Refill::v_tree_reciclador_billetes->get_model();
        single_bill_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_bill_model);
        auto list_store_bill = std::dynamic_pointer_cast<Gio::ListStore<MLevelCash>>(single_bill_selection->get_model());

        auto selection_coin_model = Global::Widget::Refill::v_tree_reciclador_monedas->get_model();
        single_coin_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_coin_model);
        auto list_store_coin = std::dynamic_pointer_cast<Gio::ListStore<MLevelCash>>(single_coin_selection->get_model());

        if (!list_store_bill || !list_store_coin)
            throw std::runtime_error("Error al obtener el modelo de la lista");
        list_store_bill->remove_all();
        for (size_t i = 0; i < level_bill->get_n_items(); i++)
        {
            list_store_bill->append(MLevelCash::create(level_bill->get_item(i)->m_denominacion,
                                                       level_bill->get_item(i)->m_cant_alm,
                                                       level_bill->get_item(i)->m_cant_recy,
                                                       level_bill->get_item(i)->m_nivel_inmo,
                                                       total_parcial_billetes[i]));
        }
        list_store_coin->remove_all();
        for (size_t i = 0; i < level_coin->get_n_items(); i++)
        {
            list_store_coin->append(MLevelCash::create(level_coin->get_item(i)->m_denominacion,
                                                       level_coin->get_item(i)->m_cant_alm,
                                                       level_coin->get_item(i)->m_cant_recy,
                                                       level_coin->get_item(i)->m_nivel_inmo,
                                                       total_parcial_monedas[i]));
        }

        single_coin_selection->unselect_all();
        single_bill_selection->unselect_all();

        calcula_total(single_bill_selection, single_coin_selection);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    } });
}

void Refill::func_poll(const std::string &status, const crow::json::rvalue &data)
{
    using namespace Global::EValidador;

    if (status == "COIN_CREDIT" ||
        status == "VALUE_ADDED" ||
        status == "ESCROW")
    {
        auto selection_coin_model = Global::Widget::Refill::v_tree_reciclador_monedas->get_model();
        single_coin_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_coin_model);
    
        auto selection_bill_model = Global::Widget::Refill::v_tree_reciclador_billetes->get_model();
        single_coin_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_bill_model);

        balance.ingreso_parcial.store(data["value"].i());
        auto ingreso = (data["value"].i() / 100);
        balance.ingreso += ingreso;
        auto select = (ingreso > 10 ? single_bill_selection : single_coin_selection);
        auto ingreso_ = (ingreso > 10 ? total_parcial_billetes : total_parcial_monedas);

        auto indice = Global::Utility::find_position((ingreso > 10 ? map_bill : map_coin), ingreso);

        try
        {
            if (indice != -1)
            {
                ingreso_[indice]++;
                select->set_selected(indice);
                if (status == "ESCROW")
                    Device::dv_bill.acepta_dinero(status, true);

                on_show_map();
            }
            else
                Global::System::showNotify("Refill",
                                           ("Deteccion de dinero no valida: " + std::to_string(ingreso) + ", Se omite registro").c_str(),
                                           "dialog-information");
        }
        catch (const std::exception &e)
        {
            std::cerr << RED << "No se pudo obtener el indice del valor indicado: " << ingreso << '\n'
                      << RESET << "Indice: " << indice << '\n';
        }
    }
}

crow::response Refill::inicia(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Carga);
    using namespace Global::EValidador;

    balance.ingreso.store(0);
    balance.cambio.store(0);
    Pago::faltante = 0;

    async_gui.dispatch_to_gui([this]()
    { 
        Global::Widget::v_main_stack->set_visible_child(*this); 
    });

    is_busy.store(true);
    is_running.store(true);

    Device::dv_coin.inicia_dispositivo_v6();
    Device::dv_bill.inicia_dispositivo_v6();

    auto future1 = std::async(std::launch::async, [this](){ Device::dv_coin.poll(sigc::mem_fun(*this, &Refill::func_poll)); });
    auto future2 = std::async(std::launch::async, [this](){ Device::dv_bill.poll(sigc::mem_fun(*this, &Refill::func_poll)); });

    future1.wait();
    future2.wait();

    Log log;
    crow::json::wvalue data;
    auto t_log = MLog::create
    (
        0,
        Global::User::id,
        "Refill",
        balance.ingreso.load(),
        0,
        balance.ingreso.load(),
        "Refill Terminado",
        Glib::DateTime::create_now_local()
    );

    t_log->m_id = log.insert_log(t_log);

    if (Global::Widget::Impresora::v_switch_impresion->get_active())
    {
        std::string command = "echo \"" + Global::System::imprime_ticket(t_log) + "\" | lp";
        std::system(command.c_str());
    }

    data = Global::Utility::json_ticket(t_log);

    data["monedas"] = v_lbl_total_parcial_monedas->get_text();
    data["billetes"] = v_lbl_total_parcial_billetes->get_text();

    data["Cambio_faltante"] = Pago::faltante;

    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child("0"); });
    is_busy.store(false);
    is_running.store(false);

    return crow::response(data);
}

crow::response Refill::get_dashboard(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Consulta_Efectivo);
    crow::json::wvalue json;

    json["bill"] = crow::json::wvalue::list();
    json["coin"] = crow::json::wvalue::list();

    auto bill_selection = Device::dv_bill.get_level_cash_actual(true,true);
    for (size_t i = 0; i < bill_selection->get_n_items(); i++)
    {
        auto m_list = bill_selection->get_typed_object<MLevelCash>(i);
        json["bill"][i]["Denominacion"] = m_list->m_denominacion;
        json["bill"][i]["Almacenado"] = m_list->m_cant_alm;
        json["bill"][i]["Recyclador"] = m_list->m_cant_recy;
        json["bill"][i]["Inmovilidad"] = m_list->m_nivel_inmo;
    }

    auto coin_selection = Device::dv_coin.get_level_cash_actual(true,true);
    for (size_t i = 0; i < coin_selection->get_n_items(); i++)
    {
        auto m_list = coin_selection->get_typed_object<MLevelCash>(i);
        json["coin"][i]["Denominacion"] = m_list->m_denominacion;
        json["coin"][i]["Almacenado"] = m_list->m_cant_alm;
        json["coin"][i]["Recyclador"] = m_list->m_cant_recy;
        json["coin"][i]["Inmovilidad"] = m_list->m_nivel_inmo;
    }

    json["total"] = v_lbl_total->get_text();
    json["total_monedas"] = v_lbl_total_monedas->get_text();
    json["total_billetes"] = v_lbl_total_billetes->get_text();

    return crow::response(json);
}

crow::response Refill::update_imovilidad(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Configuracion);
    auto data = crow::json::load(req.body);
    auto denominacion = data["denominacion"].i();
    auto nivel_inmo = data["nivel_inmo"].i();

    auto bd = denominacion > 10 ? std::make_unique<LevelCash>("Level_Bill") : std::make_unique<LevelCash>("Level_Coin");
    bd->update_nivel_inmo(denominacion, nivel_inmo);
    return crow::response();
}

crow::response Refill::transpaso(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Enviar_Casette);
    Device::dv_bill.inicia_dispositivo_v6();
    int intentos = 0;
    auto status = Device::dv_bill.reintenta_comando_post("SmartEmpty", "{"
                                                             "\"ModuleNumber\": 0,"
                                                             "\"IsNV4000\": true"
                                                             "}",
                                               intentos);
    
    auto t_log = MLog::create(0, Global::User::id, "Transpaso", 0, 0, 0, "Completado", Glib::DateTime::create_now_local());
    Log log;

    if (status.first == 200)
    {
        auto datos = Device::dv_bill.get_level_cash_actual(true);
        auto db = std::make_unique<LevelCash>("Level_Bill");
        size_t total = 0;

        for (int i = 0; i < datos->get_n_items(); i++)
        {
            auto item = datos->get_item(i);
            total += item->m_cant_recy;
            item->m_cant_alm += total;
            db->update_level_cash(item);
        }

        t_log->m_total = total;
        t_log->m_id = log.insert_log(t_log);
    }
    else
    {
        t_log->m_total = 0;
        t_log->m_estatus = "Error";
        t_log->m_id = log.insert_log(t_log);
    }
    auto data = Global::Utility::json_ticket(t_log);
    Device::dv_bill.deten_cobro_v6();

    return crow::response(status.first, data);
}

size_t Refill::saca_cassette()
{
    auto db_bill = std::make_shared<LevelCash>("Level_Bill");
    auto datos_bill = Device::dv_bill.get_level_cash_actual(true);
    size_t total_bill = 0;

    Device::dv_bill.poll([this,datos_bill,db_bill,&total_bill](const std::string &status, const crow::json::rvalue &data)
    {
        if (status == "CASHBOX_REMOVED")
        {
            for (int i = 0; i < datos_bill->get_n_items(); i++)
            {
                auto item = datos_bill->get_item(i);
                total_bill += item->m_cant_alm * item->m_denominacion;
                item->m_cant_alm = 0;
                db_bill->update_level_cash(item);
            }
            Global::EValidador::is_running.store(false);
        }
    });

    return total_bill;
}

crow::response Refill::retirada(const crow::request &req)
{
    Pago::faltante = 0;
    async_gui.dispatch_to_gui([this]()
    { Global::Widget::v_main_stack->set_visible_child(*this); });
    Device::dv_bill.inicia_dispositivo_v6();
    Device::dv_coin.inicia_dispositivo_v6();
    auto datos_coin = Device::dv_coin.get_level_cash_actual(true);
    auto status_bill = Device::dv_bill.command_post("Purge","",true);
    auto status_coin = Device::dv_coin.command_post("SmartEmpty", "{"
                                                                  "\"ModuleNumber\": 0,"
                                                                  "\"IsNV4000\": false"
                                                                  "}",true);
    
    auto t_log = MLog::create(0, Global::User::id, "Retirada", 0, 0, 0, "Completado", Glib::DateTime::create_now_local());
    Log log;

    if (status_coin.first == 200)
    {
        size_t total_coin = 0;
        
        for (size_t i = 0; i < datos_coin->get_n_items(); i++)
        {
            auto item = datos_coin->get_item(i);
            total_coin += item->m_cant_recy * item->m_denominacion;
        }
        Global::EValidador::is_running.store(true);
        size_t total_bill = saca_cassette();

        t_log->m_total = total_bill + total_coin;
    }
    else
    {
        t_log->m_total = 0;
        t_log->m_estatus = "Error";
    }

    t_log->m_id = log.insert_log(t_log);
    auto data = Global::Utility::json_ticket(t_log);

    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child("0"); });

    Device::dv_bill.deten_cobro_v6();
    Device::dv_coin.deten_cobro_v6();
    
    return crow::response(status_bill.first, data);
}

void Refill::deten()
{
    for (auto &&i : total_parcial_monedas)
        i = 0;

    for (auto &&i : total_parcial_billetes)
        i = 0; 

    Global::EValidador::is_running.store(false);
    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();
}

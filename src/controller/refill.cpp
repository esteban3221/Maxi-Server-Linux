#include "controller/refill.hpp"
#include "carrousel.hpp"

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
    Device::dv_bill.command_post("GetAllLevels");
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
                                                        level_bill->get_item(i)->m_nivel_inmo_min,
                                                        level_bill->get_item(i)->m_nivel_inmo,
                                                        level_bill->get_item(i)->m_nivel_inmo_max,
                                                        total_parcial_billetes[i]));
            }
            
            list_store_coin->remove_all();
            for (size_t i = 0; i < level_coin->get_n_items(); i++)
            {
                list_store_coin->append(MLevelCash::create(level_coin->get_item(i)->m_denominacion,
                                                        level_coin->get_item(i)->m_cant_alm,
                                                        level_coin->get_item(i)->m_cant_recy,
                                                        level_coin->get_item(i)->m_nivel_inmo_min,
                                                        level_coin->get_item(i)->m_nivel_inmo,
                                                        level_coin->get_item(i)->m_nivel_inmo_max,
                                                        total_parcial_monedas[i]));
            }

            single_coin_selection->unselect_all();
            single_bill_selection->unselect_all();

            calcula_total(single_bill_selection, single_coin_selection);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        } 
    });
}

void Refill::func_poll(const std::string &status, const crow::json::rvalue &data)
{
    if (status == "ESCROW")
    {
        auto ingreso = data["value"].i() / 100;
        auto bill_selection = Device::dv_bill.get_level_cash_actual(true);
        for (size_t i = 0; i < bill_selection->get_n_items(); i++)
        {
            if (auto m_list = bill_selection->get_typed_object<MLevelCash>(i);
                m_list->m_denominacion == ingreso)

                if (m_list->m_cant_recy +1 < m_list->m_nivel_inmo_max)
                {
                    Device::dv_bill.acepta_dinero(m_list->m_denominacion, true);
                    Device::dv_bill.command_post("AcceptFromEscrow");
                    Global::EValidador::balance.ingreso += ingreso;
                    total_parcial_billetes[i]++;
                    on_show_map();
                }
                else // if (m_list->m_cant_recy >= m_list->m_nivel_inmo_max)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    if(auto response = Device::dv_bill.command_post("ReturnFromEscrow","",true);
                        response.first == 400)
                        Global::EValidador::balance.ingreso += ingreso; // seguramente se fue a cassette
                    Device::dv_bill.acepta_dinero(m_list->m_denominacion, false);
                }
        }
    }

    if (status == "COIN_CREDIT" ||
        status == "VALUE_ADDED")
    {
        auto ingreso = data["value"].i() / 100;
        Global::EValidador::balance.ingreso += ingreso;
        total_parcial_monedas[ingreso == 1 ? 0 :
                              ingreso == 2 ? 1 :
                              ingreso == 5 ? 2 :
                              ingreso == 10 ? 3 :
                              ingreso == 20 ? 4 :
                              ingreso == 50 ? 5 : 6] ++;
        on_show_map();
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

    Device::dv_coin.inicia_dispositivo_v6(false);
    Device::dv_bill.inicia_dispositivo_v6(false);

    auto bill_selection = Device::dv_bill.get_level_cash_actual(true);
    for (size_t i = 0; i < bill_selection->get_n_items(); i++)
    {
        auto m_list = bill_selection->get_typed_object<MLevelCash>(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Simulate some delay for the device to be ready

        if (m_list->m_cant_recy < m_list->m_nivel_inmo_max)
            Device::dv_bill.acepta_dinero(m_list->m_denominacion, true);
        else //if (m_list->m_nivel_inmo_max == 0)
            Device::dv_bill.acepta_dinero(m_list->m_denominacion, false);
    }

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

    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });
    is_busy.store(false);
    is_running.store(false);

    return crow::response(data);
}

crow::response Refill::get_dashboard(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Consulta_Efectivo);
    
    auto jload = crow::json::load(req.body);
    bool refresh = true;
    crow::json::wvalue json;
    size_t total = 0,
           total_monedas = 0,
           total_billetes_cass = 0,
           total_monedas_recy = 0,
           total_billetes_recy = 0;

    json["bill"] = crow::json::wvalue::list();
    json["coin"] = crow::json::wvalue::list();

    auto bill_selection = Device::dv_bill.get_level_cash_actual(refresh);
    for (size_t i = 0; i < bill_selection->get_n_items(); i++)
    {
        auto m_list = bill_selection->get_typed_object<MLevelCash>(i);
        json["bill"][i]["Denominacion"] = m_list->m_denominacion;
        json["bill"][i]["Almacenado"] = m_list->m_cant_alm;
        json["bill"][i]["Recyclador"] = m_list->m_cant_recy;
        json["bill"][i]["Inmovilidad_Min"] = m_list->m_nivel_inmo_min;
        json["bill"][i]["Inmovilidad"] = m_list->m_nivel_inmo;
        json["bill"][i]["Inmovilidad_Max"] = m_list->m_nivel_inmo_max;

        total_billetes_cass += m_list->m_cant_alm * m_list->m_denominacion;
        total_billetes_recy += m_list->m_cant_recy * m_list->m_denominacion;
    }

    auto coin_selection = Device::dv_coin.get_level_cash_actual(refresh);
    for (size_t i = 0; i < coin_selection->get_n_items(); i++)
    {
        auto m_list = coin_selection->get_typed_object<MLevelCash>(i);
        json["coin"][i]["Denominacion"] = m_list->m_denominacion;
        json["coin"][i]["Almacenado"] = m_list->m_cant_alm;
        json["coin"][i]["Recyclador"] = m_list->m_cant_recy;
        json["coin"][i]["Inmovilidad_Min"] = m_list->m_nivel_inmo_min;
        json["coin"][i]["Inmovilidad"] = m_list->m_nivel_inmo;
        json["coin"][i]["Inmovilidad_Max"] = m_list->m_nivel_inmo_max;

        total_monedas_recy += m_list->m_cant_recy * m_list->m_denominacion;
    }

    json["total"] = std::to_string(total_billetes_cass + total_monedas + total_billetes_recy + total_monedas_recy);
    json["total_billetes_recy"] = std::to_string(total_billetes_recy);
    json["total_monedas_recy"] = std::to_string(total_monedas_recy);
    json["total_billetes_cass"] = std::to_string(total_billetes_cass);
    json["total_billetes"] = std::to_string(total_billetes_cass + total_billetes_recy);
    json["total_monedas"] = std::to_string(total_monedas);

    return crow::response(json);
}

crow::response Refill::update_imovilidad(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Configuracion);
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
    size_t total_bill = 0;
    Global::EValidador::is_running.store(true);

    auto datos_bill = Device::dv_bill.get_level_cash_actual(true);

    for (int i = 0; i < datos_bill->get_n_items(); i++)
    {
        auto item = datos_bill->get_item(i);
        total_bill += item->m_cant_alm * item->m_denominacion;
    }

    Device::dv_bill.poll([this](const std::string &status, const crow::json::rvalue &)
    {
        if (status == "CASHBOX_REMOVED")
        {
            Global::EValidador::is_running.store(false);
            Device::dv_bill.command_post("ClearNoteCashboxLevels", "", true);
        }
    });

    return total_bill;
}

crow::response Refill::retirada(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Retirada);
    Pago::faltante = 0;
    async_gui.dispatch_to_gui([this]()
    { Global::Widget::v_main_stack->set_visible_child(*this); });
    Device::dv_bill.inicia_dispositivo_v6();

    auto t_log = MLog::create(0, Global::User::id, "Retirada de Casette", 0, 0, saca_cassette(), "Completado", Glib::DateTime::create_now_local());
    Log log;
    t_log->m_id = log.insert_log(t_log);
    auto data = Global::Utility::json_ticket(t_log);

    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home); });
    Device::dv_bill.deten_cobro_v6();
    
    return crow::response(200, data);
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

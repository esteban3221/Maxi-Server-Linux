#include "controller/refill.hpp"
#include "refill.hpp"

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

    this->signal_map().connect(sigc::mem_fun(*this, &Refill::on_show));

    CROW_ROUTE(RestApp::app, "/accion/inicia_refill").methods("POST"_method)(sigc::mem_fun(*this, &Refill::inicia));
    CROW_ROUTE(RestApp::app, "/accion/deten_refill").methods("POST"_method)(sigc::mem_fun(*this, &Refill::deten));

    // ejemplo de como obtener los datos del modelo usado
    // v_tree_reciclador_monedas->signal_activate().connect([this](guint num)
    //                                                      {
    //     std::cout << "Se clicko id: " << num << '\n';
    //     auto selection_model = v_tree_reciclador_monedas->get_model();
    //     auto single_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_model);
    //
    //     //obtener la lista para añadir o eliminar varios items
    //     auto list_store = std::dynamic_pointer_cast<Gio::ListStore<MLevelCash>>(single_selection->get_model());
    //     list_store->append(MLevelCash::create(900,20,12,23));
    //
    //     //obtener item de la lista seleccionado para leer(const) o modificar dato
    //     auto m_list = single_selection->get_typed_object<MLevelCash>(num);
    //     m_list->m_nivel_inmo = num+1;
    //
    //     std::cout << "Denom " << m_list->m_denominacion <<'\n'; });
}

Refill::~Refill()
{
}

void Refill::calcula_total()
{
    auto selection_bill_model = Global::Widget::Refill::v_tree_reciclador_billetes->get_model();
    single_bill_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_bill_model);

    auto selection_coin_model = Global::Widget::Refill::v_tree_reciclador_monedas->get_model();
    single_coin_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_coin_model);

    total_bill = 0;
    total_coin = 0;
    parcial_bill = 0;
    parcial_coin = 0;

    for (size_t i = 0; i < single_bill_selection->get_n_items(); i++)
    {
        auto m_list = single_bill_selection->get_typed_object<const MLevelCash>(i);
        total_bill += m_list->m_cant_alm * m_list->m_denominacion;
        total_bill += m_list->m_cant_recy * m_list->m_denominacion;

        parcial_bill += m_list->m_ingreso * m_list->m_denominacion;
    }

    v_lbl_total_billetes->set_text(std::to_string(total_bill));
    v_lbl_total_parcial_billetes->set_text(std::to_string(parcial_bill));
    
    for (size_t i = 0; i < single_coin_selection->get_n_items(); i++)
    {
        auto m_list = single_coin_selection->get_typed_object<const MLevelCash>(i);
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

    if(tabla == "Level_Bill")
    {
        auto factory = Gtk::SignalListItemFactory::create();
        factory->signal_setup().connect(sigc::mem_fun(*this, &Refill::on_setup_label));
        factory->signal_bind().connect(sigc::mem_fun(*this, &Refill::on_bind_alm));
        auto column = Gtk::ColumnViewColumn::create("Casette", factory);
        vcolumn->append_column(column);
    }

    {
        auto factory = Gtk::SignalListItemFactory::create();
        factory->signal_setup().connect(sigc::mem_fun(*this, &Refill::on_setup_label));
        factory->signal_bind().connect(sigc::mem_fun(*this, &Refill::on_bind_recy));
        auto column = Gtk::ColumnViewColumn::create("Recylador", factory);
        vcolumn->append_column(column);
    }

    {
        auto factory = Gtk::SignalListItemFactory::create();
        factory->signal_setup().connect(sigc::mem_fun(*this, &Refill::on_setup_label));
        factory->signal_bind().connect(sigc::mem_fun(*this, &Refill::on_bind_inmo));
        auto column = Gtk::ColumnViewColumn::create("Inmovilidad", factory);
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

void Refill::on_show()
{
    try
    {
        auto level_coin = Global::Device::dv_coin.get_level_cash_actual();
        auto level_bill = Global::Device::dv_bill.get_level_cash_actual();

        auto selection_bill_model = Global::Widget::Refill::v_tree_reciclador_billetes->get_model();
        auto single_bill_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_bill_model);

        auto selection_coin_model = Global::Widget::Refill::v_tree_reciclador_monedas->get_model();
        auto single_coin_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_coin_model);

        for (size_t i = 0; i < level_bill->get_n_items(); i++)
        {
            auto m_list = single_bill_selection->get_typed_object<MLevelCash>(i);
            single_bill_selection->set_selected(i);
            m_list->m_cant_recy = level_bill->get_item(i)->m_cant_recy;
            Global::Widget::Refill::v_tree_reciclador_billetes->queue_draw();
        }

        for (size_t i = 0; i < level_coin->get_n_items(); i++)
        {
            auto m_list = single_coin_selection->get_typed_object<MLevelCash>(i);
            single_coin_selection->set_selected(i);
            m_list->m_cant_recy = level_coin->get_item(i)->m_cant_recy;
            Global::Widget::Refill::v_tree_reciclador_monedas->queue_draw();
        }

        calcula_total();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void Refill::func_poll(const std::string &status, const crow::json::rvalue &data)
{
    using namespace Global::EValidador;
    
    if (status == "COIN_CREDIT" ||
        status == "VALUE_ADDED" ||
        status == "ESCROW")
    {
        balance.ingreso_parcial.store(data["value"].i());
        auto ingreso (data["value"].i() / 100);
        calcula_total();
        auto select = (ingreso > 10 ? single_bill_selection : single_coin_selection);
        
        auto indice = Global::Utility::find_position((ingreso > 10 ? map_bill : map_coin), ingreso);
        auto m_list = select->get_typed_object<MLevelCash>(indice);

        bool recibe = m_list->m_cant_recy < m_list->m_ingreso;
        if(recibe)
        {
            m_list->m_ingreso++;
            select->select_item(indice,true);
        }
        Global::Device::dv_bill.acepta_dinero(status, recibe);
    }
}

crow::response Refill::inicia(const crow::request &req)
{
    using namespace Global::EValidador;

    balance.ingreso.store(0);
    balance.cambio.store(0);

    async_gui.dispatch_to_gui([this](){
        Global::Widget::v_main_stack->set_visible_child(*this); 
    });

    is_busy.store(true);
    is_running.store(true);

    Global::Device::dv_coin.inicia_dispositivo_v6();
    Global::Device::dv_bill.inicia_dispositivo_v6();

    auto future1 = std::async(std::launch::async, [this]() { Global::Device::dv_coin.poll(sigc::mem_fun(*this, &Refill::func_poll)); });
    auto future2 = std::async(std::launch::async, [this]() { Global::Device::dv_bill.poll(sigc::mem_fun(*this, &Refill::func_poll)); });

    future1.wait();
    future2.wait();

    return crow::response("Monedas: " + v_lbl_total_parcial_monedas->get_text() + 
                        " Billetes: " + v_lbl_total_parcial_billetes->get_text());
}

crow::response Refill::deten(const crow::request &req)
{
    Global::EValidador::is_running.store(false);
    Global::Device::dv_coin.deten_cobro_v6();
    Global::Device::dv_bill.deten_cobro_v6();
    return crow::response();
}

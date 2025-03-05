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
    v_btn_deten->signal_clicked().connect(sigc::mem_fun(*this, &Refill::deten));

    CROW_ROUTE(RestApp::app, "/accion/inicia_refill").methods("POST"_method)(sigc::mem_fun(*this, &Refill::inicia));
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
        auto level_coin = Device::dv_coin.get_level_cash_actual();
        auto level_bill = Device::dv_bill.get_level_cash_actual();

        auto selection_bill_model = Global::Widget::Refill::v_tree_reciclador_billetes->get_model();
        single_bill_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_bill_model);

        auto selection_coin_model = Global::Widget::Refill::v_tree_reciclador_monedas->get_model();
        single_coin_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_coin_model);

        for (size_t i = 0; i < level_bill->get_n_items(); i++)
        {
            auto m_list = single_bill_selection->get_typed_object<MLevelCash>(i);
            m_list->m_cant_recy = level_bill->get_item(i)->m_cant_recy;
            single_bill_selection->set_selected(i);
            m_list->m_ingreso = 0;
        }

        for (size_t i = 0; i < level_coin->get_n_items(); i++)
        {
            auto m_list = single_coin_selection->get_typed_object<MLevelCash>(i);
            m_list->m_cant_recy = level_coin->get_item(i)->m_cant_recy;
            single_coin_selection->set_selected(i);
            m_list->m_ingreso = 0;
        }

        single_coin_selection->unselect_all();
        single_bill_selection->unselect_all();

        calcula_total(single_bill_selection, single_coin_selection);
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
        auto ingreso = (data["value"].i() / 100);
        auto select = (ingreso > 10 ? single_bill_selection : single_coin_selection);
        select->unselect_all();

        auto indice = Global::Utility::find_position((ingreso > 10 ? map_bill : map_coin), ingreso);

        try
        {
            if (indice != -1)
            {
                auto m_list = select->get_typed_object<MLevelCash>(indice);

                m_list->m_ingreso++;
                select->select_item(indice, true);
                if (status == "ESCROW")
                    Device::dv_bill.acepta_dinero(status, true);

                // Hace la cuenta
                calcula_total(single_bill_selection, single_coin_selection);
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
    using namespace Global::EValidador;

    balance.ingreso.store(0);
    balance.cambio.store(0);

    async_gui.dispatch_to_gui([this]()
                              { 
        Global::Widget::v_main_stack->set_visible_child(*this); 
        v_btn_deten->set_sensitive(); });

    is_busy.store(true);
    is_running.store(true);

    Device::dv_coin.inicia_dispositivo_v6();
    Device::dv_bill.inicia_dispositivo_v6();

    auto future1 = std::async(std::launch::async, [this]()
                              { Device::dv_coin.poll(sigc::mem_fun(*this, &Refill::func_poll)); });
    auto future2 = std::async(std::launch::async, [this]()
                              { Device::dv_bill.poll(sigc::mem_fun(*this, &Refill::func_poll)); });

    future1.wait();
    future2.wait();

    async_gui.dispatch_to_gui([this]()
                              { Global::Widget::v_main_stack->set_visible_child("0"); });
    is_busy.store(false);
    is_running.store(false);

    return crow::response("Monedas: " + v_lbl_total_parcial_monedas->get_text() +
                          "\nBilletes: " + v_lbl_total_parcial_billetes->get_text());
}

void Refill::deten()
{
    async_gui.dispatch_to_gui([this]()
                              { v_btn_deten->set_sensitive(false); });
    Global::EValidador::is_running.store(false);
    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();
}

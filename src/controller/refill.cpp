#include "controller/refill.hpp"
#include "refill.hpp"

Refill::Refill(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VRefill(cobject, refBuilder),
                                                                                        total(0),
                                                                                        total_bill(0),
                                                                                        total_coin(0),
                                                                                        parcial_bill(0),
                                                                                        parcial_coin(0)
{
    init_data(Global::Widget::Refill::v_tree_reciclador_monedas, "Level_Coin");
    init_data(Global::Widget::Refill::v_tree_reciclador_billetes, "Level_Bill");

    this->signal_map().connect(sigc::mem_fun(*this, &Refill::on_show));

    // ejemplo de como obtener los datos del modelo usado
    // v_tree_reciclador_monedas->signal_activate().connect([this](guint num)
    //                                                      {
    //     std::cout << "Se clicko id: " << num << '\n';
    //     auto selection_model = v_tree_reciclador_monedas->get_model();
    //     auto single_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_model);

    //     //obtener la lista para añadir o eliminar varios items
    //     auto list_store = std::dynamic_pointer_cast<Gio::ListStore<MLevelCash>>(single_selection->get_model());
    //     list_store->append(MLevelCash::create(900,20,12,23));

    //     //obtener item de la lista seleccionado para leer(const) o modificar dato
    //     auto m_list = single_selection->get_typed_object<MLevelCash>(num);
    //     m_list->m_nivel_inmo = num+1;

    //     std::cout << "Denom " << m_list->m_denominacion <<'\n'; });
}

Refill::~Refill()
{
}

void Refill::calcula_total()
{
    auto selection_bill_model = Global::Widget::Refill::v_tree_reciclador_billetes->get_model();
    auto single_bill_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_bill_model);

    auto selection_coin_model = Global::Widget::Refill::v_tree_reciclador_monedas->get_model();
    auto single_coin_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_coin_model);

    total_bill = 0;
    total_coin = 0;
    parcial_bill = 0;
    parcial_coin = 0;

    for (size_t i = 0; i < single_bill_selection->get_n_items(); i++)
    {
        auto m_list = single_bill_selection->get_typed_object<const MLevelCash>(i);
        total_bill += m_list->m_cant_alm;
        total_bill += m_list->m_cant_recy;

        parcial_bill += m_list->m_nivel_inmo;
    }

    v_lbl_total_billetes->set_text(std::to_string(total_bill));
    v_lbl_total_parcial_billetes->set_text(std::to_string(parcial_bill));
    
    for (size_t i = 0; i < single_coin_selection->get_n_items(); i++)
    {
        auto m_list = single_coin_selection->get_typed_object<const MLevelCash>(i);
        total_coin += m_list->m_cant_alm;
        total_coin += m_list->m_cant_recy;

        parcial_coin += m_list->m_nivel_inmo;
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

void Refill::on_setup_label(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    list_item->set_child(*Gtk::make_managed<Gtk::Label>("", Gtk::Align::END));
}

void Refill::on_bind_deno(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MLevelCash>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_denominacion));
}

void Refill::on_bind_alm(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MLevelCash>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_cant_alm));
}

void Refill::on_bind_recy(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MLevelCash>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_cant_recy));
}

void Refill::on_bind_inmo(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MLevelCash>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_nivel_inmo));
}

#include "controller/refill.hpp"
#include "refill.hpp"

Refill::Refill(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VRefill(cobject, refBuilder)
{
    init_data(v_tree_reciclador_monedas,"Level_Coin");
    init_data(v_tree_reciclador_billetes,"Level_Bill");

    v_tree_reciclador_monedas->signal_activate().connect([this](guint num)
    { 
        std::cout << "Se clicko id: " << num << '\n';
        auto item = v_tree_reciclador_monedas->get_model()->get_selection(num,1);
        
    });
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

    auto factory = Gtk::SignalListItemFactory::create();
    factory->signal_setup().connect(sigc::mem_fun(*this, &Refill::on_setup_label));
    factory->signal_bind().connect(sigc::mem_fun(*this, &Refill::on_bind_deno));
    auto column = Gtk::ColumnViewColumn::create("Denominacion", factory);
    vcolumn->append_column(column);

    factory = Gtk::SignalListItemFactory::create();
    factory->signal_setup().connect(sigc::mem_fun(*this, &Refill::on_setup_label));
    factory->signal_bind().connect(sigc::mem_fun(*this, &Refill::on_bind_alm));
    column = Gtk::ColumnViewColumn::create("Almacenado", factory);
    vcolumn->append_column(column);

    column = Gtk::ColumnViewColumn::create("Recylador", factory);
    factory->signal_setup().connect(sigc::mem_fun(*this, &Refill::on_setup_label));
    factory->signal_bind().connect(sigc::mem_fun(*this, &Refill::on_bind_recy));
    vcolumn->append_column(column);

    column = Gtk::ColumnViewColumn::create("Inmobilidad", factory);
    factory->signal_setup().connect(sigc::mem_fun(*this, &Refill::on_setup_label));
    factory->signal_bind().connect(sigc::mem_fun(*this, &Refill::on_bind_inmo));
    vcolumn->append_column(column);
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

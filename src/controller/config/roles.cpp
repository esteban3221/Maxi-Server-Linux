#include "controller/config/roles.hpp"
#include "roles.hpp"

Roles::Roles(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VRoles(cobject, refBuilder)
{
    init_data();
    v_tree_usuarios->set_single_click_activate();
    v_tree_usuarios->signal_activate().connect(sigc::mem_fun(*this, &Roles::on_active_list));
}

Roles::~Roles()
{
}

void Roles::init_data()
{
    auto usuarios = std::make_unique<Usuarios>();
    auto m_list = usuarios->get_usuarios();
    auto selection_model = Gtk::SingleSelection::create(m_list);

    v_tree_usuarios->set_model(selection_model);

    auto factory = Gtk::SignalListItemFactory::create();
    factory->signal_setup().connect(sigc::mem_fun(*this, &Roles::on_setup_label));
    factory->signal_bind().connect(sigc::mem_fun(*this, &Roles::on_bind_name));

    auto column = Gtk::ColumnViewColumn::create("Usuario", factory);
    column->set_expand(true);
    v_tree_usuarios->append_column(column);
}

void Roles::on_setup_label(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    list_item->set_child(*Gtk::make_managed<Gtk::Label>("", Gtk::Align::START));
}

void Roles::on_bind_name(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MUsuarios>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_usuario));
}

void Roles::on_active_list(guint num)
{
    auto selection_model = v_tree_usuarios->get_model();
    auto single_selection = std::dynamic_pointer_cast<Gtk::SingleSelection>(selection_model);

    auto m_list = single_selection->get_typed_object<const MUsuarios>(num);

    for (auto &i : v_rol)
        i->set_active(false);

    UsuariosRoles u_roles;
    auto roles = u_roles.get_usuario_roles_by_id(m_list->m_id);

    for (size_t i = 0; i < roles->get_n_items(); i++)
    {
        auto list = roles->get_item(i);
        v_rol[list->m_id_rol - 1]->set_active();
    }
}

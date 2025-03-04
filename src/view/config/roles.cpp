#include "view/config/roles.hpp"

VRoles::VRoles(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                        Gtk::Box(cobject)
{
    for (int i = 1; i < 20; i++)
    {
        this->v_rol[i - 1] = this->m_builder->get_widget<Gtk::CheckButton>("check_rol_" + std::to_string(i));
        this->v_rol[i - 1]->set_sensitive(false);
    }
    this->v_tree_usuarios = this->m_builder->get_widget<Gtk::ColumnView>("tree_usuarios");
}

VRoles::~VRoles()
{
}

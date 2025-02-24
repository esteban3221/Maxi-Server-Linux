#include "view/config/roles.hpp"

VRoles::VRoles(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                        Gtk::Box(cobject)
{
    for (int i = 1 ; i < 20; i++)
    {
        this->rol[i] = this->m_builder->get_widget<Gtk::CheckButton>("check_rol_" + std::to_string(i));
        this->rol[i]->set_sensitive(false);
    }
    this->tree_usuarios = this->m_builder->get_widget<Gtk::ColumnView>("tree_usuarios");
}

VRoles::~VRoles()
{
}

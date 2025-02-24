#include "view/config/empresa.hpp"

VEmpresa::VEmpresa(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
Gtk::Box(cobject)
{
    list_ety_datos[0] = m_builder->get_widget<Gtk::Entry>("ety_conf_razon");
    list_ety_datos[1] = m_builder->get_widget<Gtk::Entry>("ety_conf_direccion");
    list_ety_datos[2] = m_builder->get_widget<Gtk::Entry>("ety_conf_rfc");
    list_ety_datos[3] = m_builder->get_widget<Gtk::Entry>("ety_conf_contacto");
    list_ety_datos[4] = m_builder->get_widget<Gtk::Entry>("ety_conf_thanks");
}

VEmpresa::~VEmpresa()
{
}

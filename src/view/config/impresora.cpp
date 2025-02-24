#include "view/config/impresora.hpp"

VImpresora::VImpresora(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> & refBuilder) : m_builder{refBuilder},
Gtk::Box(cobject)
{
    list_config_test = m_builder->get_widget<Gtk::ListBox>("list_config_test");
    this->list_config = m_builder->get_widget<Gtk::ListBox>("list_config");
    this->list_config_visualizacion = m_builder->get_widget<Gtk::ListBox>("list_config_visualizacion");
}

VImpresora::~VImpresora()
{
}
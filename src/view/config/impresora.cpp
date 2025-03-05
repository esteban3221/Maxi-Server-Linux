#include "view/config/impresora.hpp"

VImpresora::VImpresora(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> & refBuilder) : m_builder{refBuilder},
Gtk::Box(cobject)
{
    v_list_config_test = m_builder->get_widget<Gtk::ListBox>("list_config_test");
    v_list_config = m_builder->get_widget<Gtk::ListBox>("list_config");
    v_list_config_visualizacion = m_builder->get_widget<Gtk::ListBox>("list_config_visualizacion");

    v_switch_impresion = m_builder->get_widget<Gtk::Switch>("switch_impresion");
    v_text_ticket = m_builder->get_widget<Gtk::TextView>("text_ticket");

    for (size_t i = 1; i < 7; i++)
        v_check_config[i - 1] = m_builder->get_widget<Gtk::CheckButton>("check_config_" + std::to_string(i));
    
    
}

VImpresora::~VImpresora()
{
}
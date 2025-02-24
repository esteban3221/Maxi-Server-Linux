#include "view/config/info.hpp"

VInfo::VInfo(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                      Gtk::Box(cobject)
{
    list_info_system[0] = m_builder->get_widget<Gtk::EditableLabel>("ety_info_host");
    list_info_system[1] = m_builder->get_widget<Gtk::EditableLabel>("ety_info_modelo");
    list_info_system[2] = m_builder->get_widget<Gtk::EditableLabel>("ety_info_procesador");
    list_info_system[3] = m_builder->get_widget<Gtk::EditableLabel>("ety_info_memoria");
    list_info_system[4] = m_builder->get_widget<Gtk::EditableLabel>("ety_info_disco");
}

VInfo::~VInfo()
{
}

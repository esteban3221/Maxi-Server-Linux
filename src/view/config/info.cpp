#include "view/config/info.hpp"

VInfo::VInfo(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                      Gtk::Box(cobject)
{
    v_list_info_system[0] = m_builder->get_widget<Gtk::EditableLabel>("ety_info_host");
    v_list_info_system[1] = m_builder->get_widget<Gtk::EditableLabel>("ety_info_modelo");
    v_list_info_system[2] = m_builder->get_widget<Gtk::EditableLabel>("ety_info_procesador");
    v_list_info_system[3] = m_builder->get_widget<Gtk::EditableLabel>("ety_info_memoria");
    v_list_info_system[4] = m_builder->get_widget<Gtk::EditableLabel>("ety_info_disco");
}

VInfo::~VInfo()
{
}

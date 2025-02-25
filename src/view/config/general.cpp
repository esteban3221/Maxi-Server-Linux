#include "view/config/general.hpp"

VGeneral::VGeneral(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                            Gtk::Box(cobject)
{
    this->v_check_config_notifi = m_builder->get_widget<Gtk::CheckButton>("check_config_notifi");
    this->v_btn_select_icon = m_builder->get_widget<Gtk::Button>("btn_select_icon");
    this->v_ety_mensaje_inicio = m_builder->get_widget<Gtk::Entry>("ety_mensaje_inicio");
    this->v_btn_select_carrousel = m_builder->get_widget<Gtk::Button>("btn_select_carrousel");
    this->v_Drop_temporizador = m_builder->get_widget<Gtk::DropDown>("Drop_temporizador");
    this->v_lbl_path_carrousel = m_builder->get_widget<Gtk::Label>("lbl_path_carrousel");
    this->v_lbl_path_icon = m_builder->get_widget<Gtk::Label>("lbl_path_icon");
    this->v_list_config_general = m_builder->get_widget<Gtk::ListBox>("list_config_general");
    this->v_list_configurable = m_builder->get_widget<Gtk::ListBox>("list_configurable");

    auto list_dropdown = Gtk::StringList::create({"Ninguno",
                                                  "5 Seg.",
                                                  "10 Seg.",
                                                  "15 Seg.",
                                                  "30 Seg.",
                                                  "1 Min."});

    this->v_Drop_temporizador->property_model() = list_dropdown;
}

VGeneral::~VGeneral()
{
    
}
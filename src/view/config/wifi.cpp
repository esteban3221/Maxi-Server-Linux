#include "view/config/wifi.hpp"

VWifi::VWifi(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                      Gtk::Box(cobject)
{
    this->v_lbl_red[0] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_ip_wifi");
    this->v_lbl_red[1] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_mac_wifi");
    this->v_lbl_red[2] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_ip_eth");
    this->v_lbl_red[3] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_ip_eth_mac");
    this->v_lbl_red[4] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_ssid_wifi");

    this->v_btn_red = this->m_builder->get_widget<Gtk::Button>("btn_red");
}

VWifi::~VWifi()
{
}

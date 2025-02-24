#include "view/config/wifi.hpp"

VWifi::VWifi(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                      Gtk::Box(cobject)
{
    this->lbl_red[0] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_ip_wifi");
    this->lbl_red[1] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_mac_wifi");
    this->lbl_red[2] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_ip_eth");
    this->lbl_red[3] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_ip_eth_mac");
    this->lbl_red[4] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_ssid_wifi");
}

VWifi::~VWifi()
{
}

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

namespace View
{
    const char *ui_wifi = R"(<?xml version="1.0" encoding="UTF-8"?>
<interface>
    <requires lib="gtk" version="4.0" />
    <object class="GtkBox" id="page_5">
        <property name="orientation">1</property>
        <property name="spacing">10</property>
        <child>
            <object class="GtkLabel">
                <property name="label">Inalámbrica</property>
                <property name="margin-bottom">10</property>
                <property name="xalign">0</property>
                <style>
                    <class name="title-2" />
                </style>
            </object>
        </child>
        <child>
            <object class="GtkBox">
                <property name="homogeneous">true</property>
                <child>
                    <object class="GtkListBox" id="list_info_red">
                        <child>
                            <object class="GtkListBoxRow">
                                <property name="activatable">false</property>
                                <child>
                                    <object class="GtkBox">
                                        <child>
                                            <object class="GtkLabel">
                                                <property name="halign">1</property>
                                                <property name="label">SSID</property>
                                            </object>
                                        </child>
                                        <child>
                                            <object class="GtkEditableLabel" id="ety_info_ssid_wifi">
                                                <property name="text">Type here…</property>
                                                <property name="editable">false</property>
                                                <property name="hexpand">true</property>
                                                <property name="halign">2</property>
                                                <style>
                                                    <class name="dim-label" />
                                                </style>
                                            </object>
                                        </child>
                                    </object>
                                </child>
                            </object>
                        </child>
                        <child>
                            <object class="GtkListBoxRow">
                                <property name="activatable">false</property>
                                <child>
                                    <object class="GtkBox">
                                        <child>
                                            <object class="GtkLabel">
                                                <property name="halign">1</property>
                                                <property name="label">Direccion IP</property>
                                            </object>
                                        </child>
                                        <child>
                                            <object class="GtkEditableLabel" id="ety_info_ip_wifi">
                                                <property name="text">Type here…</property>
                                                <property name="editable">false</property>
                                                <property name="hexpand">true</property>
                                                <property name="halign">2</property>
                                                <style>
                                                    <class name="dim-label" />
                                                </style>
                                            </object>
                                        </child>
                                    </object>
                                </child>
                            </object>
                        </child>
                        <child>
                            <object class="GtkListBoxRow">
                                <property name="activatable">false</property>
                                <child>
                                    <object class="GtkBox">
                                        <child>
                                            <object class="GtkLabel">
                                                <property name="halign">1</property>
                                                <property name="label">Direccion Mac</property>
                                            </object>
                                        </child>
                                        <child>
                                            <object class="GtkEditableLabel" id="ety_info_mac_wifi">
                                                <property name="text">Type here…</property>
                                                <property name="editable">false</property>
                                                <property name="hexpand">true</property>
                                                <property name="halign">2</property>
                                                <style>
                                                    <class name="dim-label" />
                                                </style>
                                            </object>
                                        </child>
                                    </object>
                                </child>
                            </object>
                        </child>
                        <style>
                            <class name="rich-list" />
                            <class name="boxed-list" />
                        </style>
                    </object>
                </child>
            </object>
        </child>
        <child>
            <object class="GtkBox">
                <property name="orientation">1</property>
                <child>
                    <object class="GtkLabel">
                        <property name="label">Ethernet</property>
                        <property name="margin-top">20</property>
                        <property name="margin-bottom">20</property>
                        <property name="xalign">0</property>
                        <style>
                            <class name="title-2" />
                        </style>
                    </object>
                </child>
                <child>
                    <object class="GtkListBox" id="list_info_con">
                        <child>
                            <object class="GtkListBoxRow">
                                <property name="activatable">false</property>
                                <child>
                                    <object class="GtkBox">
                                        <child>
                                            <object class="GtkLabel">
                                                <property name="halign">1</property>
                                                <property name="label">Direccion IP</property>
                                            </object>
                                        </child>
                                        <child>
                                            <object class="GtkEditableLabel" id="ety_info_ip_eth">
                                                <property name="text">Type here…</property>
                                                <property name="editable">false</property>
                                                <property name="hexpand">true</property>
                                                <property name="halign">2</property>
                                                <style>
                                                    <class name="dim-label" />
                                                </style>
                                            </object>
                                        </child>
                                    </object>
                                </child>
                            </object>
                        </child>
                        <child>
                            <object class="GtkListBoxRow">
                                <property name="activatable">false</property>
                                <child>
                                    <object class="GtkBox">
                                        <child>
                                            <object class="GtkLabel">
                                                <property name="halign">1</property>
                                                <property name="label">Direccion MAC</property>
                                            </object>
                                        </child>
                                        <child>
                                            <object class="GtkEditableLabel"
                                                id="ety_info_ip_eth_mac">
                                                <property name="text">Type here…</property>
                                                <property name="editable">false</property>
                                                <property name="hexpand">true</property>
                                                <property name="halign">2</property>
                                                <style>
                                                    <class name="dim-label" />
                                                </style>
                                            </object>
                                        </child>
                                    </object>
                                </child>
                            </object>
                        </child>
                        <style>
                            <class name="rich-list" />
                            <class name="boxed-list" />
                        </style>
                    </object>
                </child>
                <child>
                    <object class="GtkButton" id="btn_red">
                        <property name="label">Configuración</property>
                        <property name="margin-top">40</property>
                        <property name="halign">3</property>
                        <style>
                            <class name="suggested-action" />
                        </style>
                    </object>
                </child>
            </object>
        </child>
    </object>
</interface>)";
}
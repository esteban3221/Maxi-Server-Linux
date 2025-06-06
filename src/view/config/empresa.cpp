#include "view/config/empresa.hpp"

VEmpresa::VEmpresa(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
Gtk::Box(cobject)
{
    v_list_ety_datos[0] = m_builder->get_widget<Gtk::Entry>("ety_conf_razon");
    v_list_ety_datos[1] = m_builder->get_widget<Gtk::Entry>("ety_conf_direccion");
    v_list_ety_datos[2] = m_builder->get_widget<Gtk::Entry>("ety_conf_rfc");
    v_list_ety_datos[3] = m_builder->get_widget<Gtk::Entry>("ety_conf_contacto");
    v_list_ety_datos[4] = m_builder->get_widget<Gtk::Entry>("ety_conf_thanks");
}

VEmpresa::~VEmpresa()
{
}

namespace View
{
    const char *ui_empresa = R"(<?xml version="1.0" encoding="UTF-8"?>
<interface>
    <object class="GtkBox" id="page_2">
        <property name="orientation">1</property>
        <child>
            <object class="GtkListBox" id="list_config_datos">
                <property name="selection-mode">0</property>
                <child>
                    <object class="GtkListBoxRow">
                        <property name="activatable">false</property>
                        <child>
                            <object class="GtkBox">
                                <child>
                                    <object class="GtkLabel" id="entry_label_7">
                                        <property name="xalign">0</property>
                                        <property name="width-request">110</property>
                                        <property name="label">Razón social</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkEntry" id="ety_conf_razon">
                                        <property name="placeholder-text">Type here…</property>
                                        <property name="hexpand">true</property>
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
                                    <object class="GtkLabel" id="entry_label_3">
                                        <property name="xalign">0</property>
                                        <property name="width-request">110</property>
                                        <property name="label">Direccion</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkEntry" id="ety_conf_direccion">
                                        <property name="placeholder-text">Type here…</property>
                                        <property name="hexpand">true</property>
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
                                    <object class="GtkLabel" id="entry_label">
                                        <property name="xalign">0</property>
                                        <property name="width-request">110</property>
                                        <property name="label">RFC</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkEntry" id="ety_conf_rfc">
                                        <property name="placeholder-text">Type here…</property>
                                        <property name="hexpand">true</property>
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
                                    <object class="GtkLabel" id="entry_label_4">
                                        <property name="xalign">0</property>
                                        <property name="width-request">110</property>
                                        <property name="label">Contacto</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkEntry" id="ety_conf_contacto">
                                        <property name="placeholder-text">Type here…</property>
                                        <property name="hexpand">true</property>
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
                                    <object class="GtkLabel" id="entry_label_5">
                                        <property name="xalign">0</property>
                                        <property name="width-request">110</property>
                                        <property name="label">Agradecimiento</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkEntry" id="ety_conf_thanks">
                                        <property name="placeholder-text">Type here…</property>
                                        <property name="hexpand">true</property>
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
</interface>)";
}

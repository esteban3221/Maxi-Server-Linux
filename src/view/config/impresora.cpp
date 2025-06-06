#include "view/config/impresora.hpp"

VImpresora::VImpresora(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> & refBuilder) : m_builder{refBuilder},
Gtk::Box(cobject)
{
    v_list_config_test = m_builder->get_widget<Gtk::ListBox>("list_config_test");
    v_list_config = m_builder->get_widget<Gtk::ListBox>("list_config");
    v_list_config_visualizacion = m_builder->get_widget<Gtk::ListBox>("list_config_visualizacion");

    Global::Widget::Impresora::v_switch_impresion = m_builder->get_widget<Gtk::Switch>("switch_impresion");
    v_text_ticket = m_builder->get_widget<Gtk::TextView>("text_ticket");

    for (size_t i = 1; i < 7; i++)
        v_check_config[i - 1] = m_builder->get_widget<Gtk::CheckButton>("check_config_" + std::to_string(i));
    
    
}

VImpresora::~VImpresora()
{
}

namespace View
{
    const char *ui_impresora = R"(<?xml version="1.0" encoding="UTF-8"?>
<interface>
    <object class="GtkBox" id="page_3">
        <child>
            <object class="GtkScrolledWindow">
                <property name="hexpand">false</property>
                <property name="hscrollbar-policy">2</property>
                <property name="vexpand">true</property>
                <child>
                    <object class="GtkViewport">
                        <property name="scroll-to-focus">true</property>
                        <child>
                            <object class="GtkBox">
                                <property name="margin-bottom">30</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="orientation">1</property>
                                <child>
                                    <object class="GtkLabel">
                                        <property name="label">Activar</property>
                                        <property name="margin-bottom">10</property>
                                        <property name="xalign">0</property>
                                        <style>
                                            <class name="title-2" />
                                        </style>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkListBox" id="list_config">
                                        <property name="selection-mode">0</property>
                                        <property name="margin-bottom">30</property>
                                        <child>
                                            <object class="GtkListBoxRow">
                                                <child>
                                                    <object class="GtkBox">
                                                        <child>
                                                            <object class="GtkLabel"
                                                                id="entry_label_activar">
                                                                <property name="halign">1</property>
                                                                <property name="hexpand">true</property>
                                                                <property name="label">Activar Impresión</property>
                                                                <property name="valign">3</property>
                                                                <property name="xalign">0</property>
                                                            </object>
                                                        </child>
                                                        <child>
                                                            <object class="GtkSwitch" id="switch_impresion">
                                                                <property name="halign">2</property>
                                                                <property name="valign">3</property>
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
                                    <object class="GtkLabel">
                                        <property name="label">Visualización</property>
                                        <property name="margin-bottom">10</property>
                                        <property name="xalign">0</property>
                                        <style>
                                            <class name="title-2" />
                                        </style>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkListBox" id="list_config_visualizacion">
                                        <property name="selection-mode">0</property>
                                        <child>
                                            <object class="GtkListBoxRow">
                                                <child>
                                                    <object class="GtkBox">
                                                        <child>
                                                            <object class="GtkLabel"
                                                                id="switch_label">
                                                                <property name="halign">1</property>
                                                                <property name="hexpand">true</property>
                                                                <property name="label">Mensaje de Agradecimiento</property>
                                                                <property name="valign">3</property>
                                                                <property name="xalign">0</property>
                                                            </object>
                                                        </child>
                                                        <child>
                                                            <object class="GtkCheckButton"
                                                                id="check_config_1">
                                                                <property name="active">true</property>
                                                                <property name="halign">2</property>
                                                                <property name="margin-end">10</property>
                                                                <property name="margin-start">10</property>
                                                                <property name="valign">3</property>
                                                            </object>
                                                        </child>
                                                    </object>
                                                </child>
                                            </object>
                                        </child>
                                        <child>
                                            <object class="GtkListBoxRow">
                                                <child>
                                                    <object class="GtkBox">
                                                        <child>
                                                            <object class="GtkLabel" id="check_label">
                                                                <property name="halign">1</property>
                                                                <property name="hexpand">true</property>
                                                                <property name="label">Fecha</property>
                                                                <property name="valign">3</property>
                                                                <property name="xalign">0</property>
                                                            </object>
                                                        </child>
                                                        <child>
                                                            <object class="GtkCheckButton"
                                                                id="check_config_2">
                                                                <property name="active">true</property>
                                                                <property name="halign">2</property>
                                                                <property name="margin-end">10</property>
                                                                <property name="margin-start">10</property>
                                                                <property name="valign">3</property>
                                                            </object>
                                                        </child>
                                                    </object>
                                                </child>
                                            </object>
                                        </child>
                                        <child>
                                            <object class="GtkListBoxRow">
                                                <child>
                                                    <object class="GtkBox">
                                                        <child>
                                                            <object class="GtkLabel"
                                                                id="check_label_2">
                                                                <property name="halign">1</property>
                                                                <property name="hexpand">true</property>
                                                                <property name="label">Direccion</property>
                                                                <property name="valign">3</property>
                                                                <property name="xalign">0</property>
                                                            </object>
                                                        </child>
                                                        <child>
                                                            <object class="GtkCheckButton"
                                                                id="check_config_3">
                                                                <property name="active">true</property>
                                                                <property name="halign">2</property>
                                                                <property name="margin-end">10</property>
                                                                <property name="margin-start">10</property>
                                                                <property name="valign">3</property>
                                                            </object>
                                                        </child>
                                                    </object>
                                                </child>
                                            </object>
                                        </child>
                                        <child>
                                            <object class="GtkListBoxRow">
                                                <child>
                                                    <object class="GtkBox">
                                                        <child>
                                                            <object class="GtkLabel"
                                                                id="check_label_3">
                                                                <property name="halign">1</property>
                                                                <property name="hexpand">true</property>
                                                                <property name="label">RFC</property>
                                                                <property name="valign">3</property>
                                                                <property name="xalign">0</property>
                                                            </object>
                                                        </child>
                                                        <child>
                                                            <object class="GtkCheckButton"
                                                                id="check_config_4">
                                                                <property name="active">true</property>
                                                                <property name="halign">2</property>
                                                                <property name="margin-end">10</property>
                                                                <property name="margin-start">10</property>
                                                                <property name="valign">3</property>
                                                            </object>
                                                        </child>
                                                    </object>
                                                </child>
                                            </object>
                                        </child>
                                        <child>
                                            <object class="GtkListBoxRow">
                                                <child>
                                                    <object class="GtkBox">
                                                        <child>
                                                            <object class="GtkLabel"
                                                                id="check_label_4">
                                                                <property name="halign">1</property>
                                                                <property name="hexpand">true</property>
                                                                <property name="label">Vendedor</property>
                                                                <property name="valign">3</property>
                                                                <property name="xalign">0</property>
                                                            </object>
                                                        </child>
                                                        <child>
                                                            <object class="GtkCheckButton"
                                                                id="check_config_5">
                                                                <property name="active">true</property>
                                                                <property name="halign">2</property>
                                                                <property name="margin-end">10</property>
                                                                <property name="margin-start">10</property>
                                                                <property name="valign">3</property>
                                                            </object>
                                                        </child>
                                                    </object>
                                                </child>
                                            </object>
                                        </child>
                                        <child>
                                            <object class="GtkListBoxRow">
                                                <child>
                                                    <object class="GtkBox">
                                                        <child>
                                                            <object class="GtkLabel"
                                                                id="check_label_5">
                                                                <property name="halign">1</property>
                                                                <property name="hexpand">true</property>
                                                                <property name="label">Contacto</property>
                                                                <property name="valign">3</property>
                                                                <property name="xalign">0</property>
                                                            </object>
                                                        </child>
                                                        <child>
                                                            <object class="GtkCheckButton"
                                                                id="check_config_6">
                                                                <property name="active">true</property>
                                                                <property name="halign">2</property>
                                                                <property name="margin-end">10</property>
                                                                <property name="margin-start">10</property>
                                                                <property name="valign">3</property>
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
                                    <object class="GtkLabel">
                                        <property name="label">Test</property>
                                        <property name="margin-bottom">10</property>
                                        <property name="margin-top">30</property>
                                        <property name="xalign">0</property>
                                        <style>
                                            <class name="title-2" />
                                        </style>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkListBox" id="list_config_test">
                                        <property name="selection-mode">0</property>
                                        <child>
                                            <object class="GtkListBoxRow">
                                                <child>
                                                    <object class="GtkBox">
                                                        <child>
                                                            <object class="GtkLabel"
                                                                id="entry_label_8">
                                                                <property name="halign">1</property>
                                                                <property name="hexpand">true</property>
                                                                <property name="label">Impresion de Prueba</property>
                                                                <property name="valign">3</property>
                                                                <property name="xalign">0</property>
                                                            </object>
                                                        </child>
                                                        <child>
                                                            <object class="GtkImage">
                                                                <property name="halign">2</property>
                                                                <property name="icon-name">
                                                                    printer-symbolic</property>
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
                    </object>
                </child>
            </object>
        </child>
        <child>
            <object class="GtkScrolledWindow">
                <property name="hexpand">true</property>
                <property name="margin-bottom">20</property>
                <property name="margin-end">10</property>
                <property name="margin-start">30</property>
                <property name="margin-top">20</property>
                <child>
                    <object class="GtkTextView" id="text_ticket">
                        <property name="cursor-visible">false</property>
                        <property name="editable">false</property>
                        <property name="input-purpose">9</property>
                        <property name="left-margin">5</property>
                        <property name="monospace">true</property>
                        <property name="pixels-above-lines">5</property>
                        <property name="pixels-below-lines">5</property>
                        <property name="pixels-inside-wrap">5</property>
                        <property name="right-margin">5</property>
                        <property name="top-margin">5</property>
                    </object>
                </child>
            </object>
        </child>
    </object>
</interface>)";
}
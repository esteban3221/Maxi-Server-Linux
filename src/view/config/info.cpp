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

namespace View
{
    const char *ui_info = R"(<?xml version="1.0" encoding="UTF-8"?>
<interface>
    <object class="GtkBox" id="page_1">
        <property name="orientation">1</property>
        <child>
            <object class="GtkListBox" id="list_info_systema">
                <child>
                    <object class="GtkListBoxRow">
                        <property name="activatable">false</property>
                        <child>
                            <object class="GtkBox">
                                <child>
                                    <object class="GtkLabel">
                                        <property name="halign">1</property>
                                        <property name="label">Nombre del Dispositivo</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkEditableLabel" id="ety_info_host">
                                        <property name="text">Type here…</property>
                                        <property name="editable">true</property>
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
                                    <object class="GtkLabel" id="entry_">
                                        <property name="halign">1</property>
                                        <property name="label">Modelo de Hardware</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkEditableLabel" id="ety_info_modelo">
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
                                        <property name="label">Procesador</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkEditableLabel" id="ety_info_procesador">
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
                                        <property name="label">Memoria</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkEditableLabel" id="ety_info_memoria">
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
                                        <property name="label">Disco</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkEditableLabel" id="ety_info_disco">
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
</interface>)";
}

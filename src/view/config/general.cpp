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

namespace View
{
    const char *ui_general = R"(<?xml version="1.0" encoding="UTF-8"?>
<interface>
    <object class="GtkBox" id="page_0">
        <property name="orientation">1</property>
        <property name="spacing">10</property>
        <child>
            <object class="GtkListBox" id="list_configurable">
                <child>
                    <object class="GtkListBoxRow">
                        <child>
                            <object class="GtkBox">
                                <child>
                                    <object class="GtkLabel">
                                        <property name="halign">1</property>
                                        <property name="hexpand">true</property>
                                        <property name="label">Mostrar Notificaciones</property>
                                        <property name="valign">3</property>
                                        <property name="xalign">0</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkCheckButton" id="check_config_notifi">
                                        <property name="active">true</property>
                                        <property name="halign">2</property>
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
                                    <object class="GtkLabel">
                                        <property name="halign">1</property>
                                        <property name="hexpand">true</property>
                                        <property name="label">Logo Inicio</property>
                                        <property name="valign">3</property>
                                        <property name="xalign">0</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkButton" id="btn_select_icon">
                                        <property name="halign">2</property>
                                        <property name="valign">3</property>
                                        <property name="child">
                                            <object class="GtkBox">
                                                <property name="spacing">5</property>
                                                <child>
                                                    <object class="GtkImage">
                                                        <property name="icon-name">
                                                            insert-image-symbolic</property>
                                                    </object>
                                                </child>
                                                <child>
                                                    <object class="GtkLabel" id="lbl_path_icon">
                                                        <property name="label">Selecciona Imagen</property>
                                                        <property name="ellipsize">1</property>
                                                    </object>
                                                </child>
                                            </object>
                                        </property>
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
                                    <object class="GtkLabel">
                                        <property name="halign">1</property>
                                        <property name="hexpand">true</property>
                                        <property name="label">Mensaje de Inicio</property>
                                        <property name="valign">3</property>
                                        <property name="xalign">0</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkEntry" id="ety_mensaje_inicio">
                                        <property name="halign">2</property>
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
                                    <object class="GtkLabel">
                                        <property name="halign">1</property>
                                        <property name="hexpand">true</property>
                                        <property name="label">Carrusel de imágenes</property>
                                        <property name="valign">3</property>
                                        <property name="xalign">0</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkButton" id="btn_select_carrousel">
                                        <property name="halign">2</property>
                                        <property name="margin-end">10</property>
                                        <property name="valign">3</property>
                                        <property name="child">
                                            <object class="GtkBox">
                                                <property name="spacing">5</property>
                                                <child>
                                                    <object class="GtkImage">
                                                        <property name="icon-name">
                                                            inode-directory-symbolic</property>
                                                    </object>
                                                </child>
                                                <child>
                                                    <object class="GtkLabel" id="lbl_path_carrousel">
                                                        <property name="label">Selecciona Carpeta</property>
                                                        <property name="ellipsize">1</property>
                                                    </object>
                                                </child>
                                            </object>
                                        </property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkDropDown" id="Drop_temporizador"></object>
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
            <object class="GtkListBox" id="list_config_general">
                <property name="selection-mode">0</property>
                <child>
                    <object class="GtkListBoxRow">
                        <child>
                            <object class="GtkBox">
                                <child>
                                    <object class="GtkLabel">
                                        <property name="halign">1</property>
                                        <property name="hexpand">true</property>
                                        <property name="label">Establecer PIN de acceso</property>
                                        <property name="valign">3</property>
                                        <property name="xalign">0</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkImage">
                                        <property name="halign">2</property>
                                        <property name="icon-name">input-dialpad-symbolic</property>
                                        <property name="margin-end">10</property>
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
                                    <object class="GtkLabel">
                                        <property name="halign">1</property>
                                        <property name="hexpand">true</property>
                                        <property name="label">Reiniciar</property>
                                        <property name="valign">3</property>
                                        <property name="xalign">0</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkImage">
                                        <property name="halign">2</property>
                                        <property name="icon-name">system-reboot-symbolic</property>
                                        <property name="margin-end">10</property>
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
                                    <object class="GtkLabel">
                                        <property name="halign">1</property>
                                        <property name="hexpand">true</property>
                                        <property name="label">Apagar</property>
                                        <property name="valign">3</property>
                                        <property name="xalign">0</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkImage">
                                        <property name="halign">2</property>
                                        <property name="icon-name">system-shutdown-symbolic</property>
                                        <property name="margin-end">10</property>
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
                                    <object class="GtkLabel">
                                        <property name="halign">1</property>
                                        <property name="hexpand">true</property>
                                        <property name="label">Restablecer de Fabrica</property>
                                        <property name="valign">3</property>
                                        <property name="xalign">0</property>
                                    </object>
                                </child>
                                <child>
                                    <object class="GtkImage">
                                        <property name="halign">2</property>
                                        <property name="icon-name">computer-fail-symbolic</property>
                                        <property name="margin-end">10</property>
                                    </object>
                                </child>
                            </object>
                        </child>
                        <style>
                            <class name="warning" />
                            <class name="activatable" />
                        </style>
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

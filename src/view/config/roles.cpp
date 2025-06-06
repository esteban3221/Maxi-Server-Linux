#include "view/config/roles.hpp"

VRoles::VRoles(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                        Gtk::Box(cobject)
{
    for (int i = 1; i < 20; i++)
    {
        this->v_rol[i - 1] = this->m_builder->get_widget<Gtk::CheckButton>("check_rol_" + std::to_string(i));
        this->v_rol[i - 1]->set_sensitive(false);
    }
    this->v_tree_usuarios = this->m_builder->get_widget<Gtk::ColumnView>("tree_usuarios");
}

VRoles::~VRoles()
{
}

namespace View
{
    const char *ui_roles = R"(<?xml version="1.0" encoding="UTF-8"?>
<interface>
    <object class="GtkBox" id="page_4">
        <property name="spacing">10</property>
        <child>
            <object class="GtkScrolledWindow">
                <child>
                    <object class="GtkColumnView" id="tree_usuarios">
                        <!-- <property name="enable-grid-lines">2</property>
                        <property name="enable-search">false</property> -->
                        <property name="hexpand">true</property>
                        <property name="vexpand">true</property>
                    </object>
                </child>
            </object>
        </child>
        <child>
            <object class="GtkFrame">
                <property name="label">Permisos</property>
                <child>
                    <object class="GtkGrid">
                        <property name="column-homogeneous">true</property>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_1">
                                <property name="label">Venta</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">0</property>
                                    <property name="row">0</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_2">
                                <property name="label">Pago</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">0</property>
                                    <property name="row">1</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_3">
                                <property name="label">Carga</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">0</property>
                                    <property name="row">2</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_4">
                                <property name="label">Retirada</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">0</property>
                                    <property name="row">3</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_5">
                                <property name="label">Cambio Manual</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">0</property>
                                    <property name="row">4</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_6">
                                <property name="label">Cambio Automatico</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">0</property>
                                    <property name="row">5</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_7">
                                <property name="label">Ingresos</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">0</property>
                                    <property name="row">6</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_8">
                                <property name="label">Enviar a casette</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">1</property>
                                    <property name="row">0</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_9">
                                <property name="label">Retirada Casette</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">1</property>
                                    <property name="row">1</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_10">
                                <property name="label">Consulta de Efectivo</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">1</property>
                                    <property name="row">2</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_11">
                                <property name="label">Mov. Pendientes</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">1</property>
                                    <property name="row">3</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_12">
                                <property name="label">Consulta Movimiento</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">1</property>
                                    <property name="row">4</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_13">
                                <property name="label">Cierre con Faltantes</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">1</property>
                                    <property name="row">5</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkSeparator">
                                <layout>
                                    <property name="column">0</property>
                                    <property name="row">8</property>
                                    <property name="column-span">2</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_14">
                                <property name="label">Estadisticas</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">0</property>
                                    <property name="row">9</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_15">
                                <property name="label">Fianza</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">0</property>
                                    <property name="row">10</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_16">
                                <property name="label">Mostrar Reportes</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">0</property>
                                    <property name="row">11</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_17">
                                <property name="label">Configuracion</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">1</property>
                                    <property name="row">9</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_18">
                                <property name="label">Salir a Windows</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">1</property>
                                    <property name="row">10</property>
                                </layout>
                            </object>
                        </child>
                        <child>
                            <object class="GtkCheckButton" id="check_rol_19">
                                <property name="label">Apagar el equipo</property>
                                <property name="halign">1</property>
                                <property name="margin-end">10</property>
                                <property name="margin-start">10</property>
                                <property name="valign">3</property>
                                <layout>
                                    <property name="column">1</property>
                                    <property name="row">11</property>
                                </layout>
                            </object>
                        </child>
                    </object>
                </child>
            </object>
        </child>
    </object>
</interface>)";
}
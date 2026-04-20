#include "view/metodo_pago.hpp"

VMetodoPago::VMetodoPago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                                        Gtk::Box(cobject)
{
    v_btn_cash = m_builder->get_widget<Gtk::Button>("btn_cash");
    v_btn_card = m_builder->get_widget<Gtk::Button>("btn_card");
    v_btn_deferred = m_builder->get_widget<Gtk::Button>("btn_deferred");
}

VMetodoPago::~VMetodoPago()
{
}

namespace View
{
    const char *ui_metodo_pago = R"(
    <?xml version="1.0" encoding="UTF-8"?>
<interface>
  <requires lib="gtk" version="4.0"/>
  <template class="PaymentView" parent="GtkBox">
    <property name="orientation">1</property>
    <property name="halign">3</property>
    <property name="valign">3</property>
    <property name="spacing">24</property>
    <property name="margin-top">32</property>
    <property name="margin-bottom">32</property>
    <property name="margin-start">32</property>
    <property name="margin-end">32</property>
    <child>
      <object class="GtkLabel">
        <property name="label">Seleccione el método de pago</property>
        <style>
          <class name="title-1"/>
        </style>
      </object>
    </child>
    <child>
      <object class="GtkGrid">
        <property name="column-spacing">12</property>
        <property name="row-spacing">12</property>
        <property name="halign">3</property>
        <property name="column-homogeneous">true</property>
        <property name="row-homogeneous">true</property>
        <child>
          <object class="GtkButton" id="btn_cash">
            <layout>
              <property name="column">0</property>
              <property name="row">0</property>
            </layout>
            <style>
              <class name="flat"/>
            </style>
            <child>
              <object class="GtkBox">
                <property name="orientation">1</property>
                <property name="spacing">8</property>
                <property name="margin-top">20</property>
                <property name="margin-bottom">20</property>
                <child>
                  <object class="GtkImage">
                    <property name="icon-name">money-symbolic</property>
                    <property name="pixel-size">32</property>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel">
                    <property name="label">Efectivo</property>
                  </object>
                </child>
              </object>
            </child>
          </object>
        </child>
        <child>
          <object class="GtkButton" id="btn_card">
            <layout>
              <property name="column">1</property>
              <property name="row">0</property>
            </layout>
            <style>
              <class name="flat"/>
            </style>
            <child>
              <object class="GtkBox">
                <property name="orientation">1</property>
                <property name="spacing">8</property>
                <property name="margin-top">20</property>
                <property name="margin-bottom">20</property>
                <child>
                  <object class="GtkImage">
                    <property name="icon-name">phonelink-ring-symbolic</property>
                    <property name="pixel-size">32</property>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel">
                    <property name="label">Tarjeta</property>
                  </object>
                </child>
              </object>
            </child>
          </object>
        </child>
        <child>
          <object class="GtkButton" id="btn_deferred">
            <layout>
              <property name="column">0</property>
              <property name="row">1</property>
              <property name="column-span">2</property>
            </layout>
            <style>
              <class name="flat"/>
            </style>
            <child>
              <object class="GtkBox">
                <property name="orientation">0</property>
                <property name="halign">3</property>
                <property name="spacing">16</property>
                <property name="margin-top">15</property>
                <property name="margin-bottom">15</property>
                <child>
                  <object class="GtkImage">
                    <property name="icon-name">shape-balance-symbolic</property>
                    <property name="pixel-size">32</property>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel">
                    <property name="label">Diferir Pago</property>
                  </object>
                </child>
              </object>
            </child>
          </object>
        </child>
      </object>
    </child>
  </template>
</interface>
    )";
}
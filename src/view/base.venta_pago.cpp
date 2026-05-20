#include "view/base.venta_pago.hpp"


BVentaPago::BVentaPago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                                Gtk::Box(cobject)
{
    v_lbl_cambio = m_builder->get_widget<Gtk::Label>("lbl_cambio");
    v_lbl_faltante = m_builder->get_widget<Gtk::Label>("lbl_faltante");
    v_lbl_mensaje_fin = m_builder->get_widget<Gtk::Label>("lbl_mensaje_fin");
    v_lbl_monto_total = m_builder->get_widget<Gtk::Label>("lbl_monto_total");
    v_lbl_recibido = m_builder->get_widget<Gtk::Label>("lbl_recibido");
    v_lbl_titulo = m_builder->get_widget<Gtk::Label>("lbl_titulo");
    v_img_main = m_builder->get_widget<Gtk::Image>("img_main");

    v_BXRW4 = m_builder->get_widget<Gtk::ListBoxRow>("BXRW4");

    v_btn_timeout_cancel = m_builder->get_widget<Gtk::Button>("btn_timeout_cancel");
    v_btn_timeout_retry = m_builder->get_widget<Gtk::Button>("btn_timeout_retry");

    v_btn_timeout_cancel->signal_clicked().connect(sigc::mem_fun(*this, &BVentaPago::on_btn_cancel_click));
    v_btn_timeout_retry->signal_clicked().connect(sigc::mem_fun(*this, &BVentaPago::on_btn_retry_click));
}

BVentaPago::~BVentaPago()
{
}

namespace View
{
    const char *ui_vp = R"(
    <?xml version="1.0" encoding="UTF-8"?>
<interface>
  <requires lib="gtk" version="4.0"/>
  <object class="GtkBox" id="box">
    <property name="margin-bottom">30</property>
    <property name="margin-end">40</property>
    <property name="margin-start">40</property>
    <property name="margin-top">30</property>
    <property name="orientation">1</property>
    <property name="spacing">24</property>
    <property name="vexpand">true</property>
    <child>
      <object class="GtkBox">
        <property name="orientation">1</property>
        <property name="spacing">8</property>
        <child>
          <object class="GtkImage" id="img_main">
            <property name="icon-name">emblem-system-symbolic</property>
            <property name="pixel-size">48</property>
            <style>
              <class name="dim-label"/>
            </style>
          </object>
        </child>
        <child>
          <object class="GtkLabel" id="lbl_titulo">
            <property name="label">Resumen de Pago</property>
            <style>
              <class name="title-1"/>
            </style>
          </object>
        </child>
      </object>
    </child>
    <child>
      <object class="GtkListBox" id="list_venta">
        <property name="selection-mode">0</property>
        <style>
          <class name="boxed-list"/>
        </style>
        <child>
          <object class="GtkListBoxRow" id="BXRW1">
            <property name="activatable">false</property>
            <child>
              <object class="GtkBox">
                <property name="margin-start">15</property>
                <property name="margin-end">15</property>
                <property name="margin-top">15</property>
                <property name="margin-bottom">15</property>
                <child>
                  <object class="GtkLabel">
                    <property name="label">Total a Pagar</property>
                    <style>
                      <class name="heading"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_monto_total">
                    <property name="halign">2</property>
                    <property name="hexpand">true</property>
                    <style>
                      <class name="title-1"/>
                      <class name="accent"/>
                    </style>
                  </object>
                </child>
              </object>
            </child>
          </object>
        </child>
        <child>
          <object class="GtkListBoxRow" id="BXRW2">
            <child>
              <object class="GtkBox">
                <property name="margin-start">15</property>
                <property name="margin-end">15</property>
                <property name="margin-top">10</property>
                <property name="margin-bottom">10</property>
                <child>
                  <object class="GtkLabel">
                    <property name="label">Recibido</property>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_recibido">
                    <property name="halign">2</property>
                    <property name="hexpand">true</property>
                    <style>
                      <class name="title-3"/>
                      <class name="success"/>
                    </style>
                  </object>
                </child>
              </object>
            </child>
          </object>
        </child>
        <child>
          <object class="GtkListBoxRow" id="BXRW3">
            <child>
              <object class="GtkBox">
                <property name="margin-start">15</property>
                <property name="margin-end">15</property>
                <property name="margin-top">10</property>
                <property name="margin-bottom">10</property>
                <child>
                  <object class="GtkLabel">
                    <property name="label">Faltante</property>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_faltante">
                    <property name="halign">2</property>
                    <property name="hexpand">true</property>
                    <style>
                      <class name="title-3"/>
                      <class name="error"/>
                    </style>
                  </object>
                </child>
              </object>
            </child>
          </object>
        </child>
        <child>
          <object class="GtkListBoxRow" id="BXRW4">
            <child>
              <object class="GtkBox">
                <property name="margin-start">15</property>
                <property name="margin-end">15</property>
                <property name="margin-top">10</property>
                <property name="margin-bottom">10</property>
                <child>
                  <object class="GtkLabel">
                    <property name="label">Cambio</property>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_cambio">
                    <property name="halign">2</property>
                    <property name="hexpand">true</property>
                    <style>
                      <class name="title-3"/>
                      <class name="dim-label"/>
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
      <object class="GtkLabel" id="lbl_mensaje_fin">
        <property name="visible">false</property>
        <property name="wrap">true</property>
        <property name="justify">2</property>
        <style>
          <class name="title-2"/>
          <class name="success"/>
        </style>
      </object>
    </child>
    <child>
      <object class="GtkBox">
        <property name="orientation">1</property>
        <property name="valign">2</property>
        <property name="vexpand">true</property>
        <property name="spacing">16</property>
        <child>
          <object class="GtkBox" id="box_action_timeout">
            <property name="halign">3</property>
            <property name="spacing">12</property>
            <child>
              <object class="GtkButton" id="btn_timeout_cancel">
                <property name="label">Cancelar Pago</property>
                <style>
                  <class name="destructive-action"/>
                  <class name="pill"/>
                </style>
              </object>
            </child>
            <child>
              <object class="GtkButton" id="btn_timeout_retry">
                <property name="label">Añadir Tiempo</property>
                <style>
                  <class name="suggested-action"/>
                  <class name="pill"/>
                </style>
              </object>
            </child>
          </object>
        </child>
      </object>
    </child>
  </object>
</interface>
    )";
}
#include "view/base.venta_pago.hpp"


BVentaPago::BVentaPago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                                Gtk::Box(cobject)
{
    v_lbl_cambio = m_builder->get_widget<Gtk::Label>("lbl_cambio");
    v_lbl_faltante = m_builder->get_widget<Gtk::Label>("lbl_faltante");
    v_lbl_mensaje_fin = m_builder->get_widget<Gtk::Label>("lbl_mensaje_fin");
    v_lbl_monto_total = m_builder->get_widget<Gtk::Label>("lbl_monto_total");
    v_lbl_recibido = m_builder->get_widget<Gtk::Label>("lbl_recibido");
    v_lbl_timeout = m_builder->get_widget<Gtk::Label>("lbl_timeout");
    v_lbl_titulo = m_builder->get_widget<Gtk::Label>("lbl_titulo");

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
    const char *ui_vp = R"(<?xml version='1.0' encoding='UTF-8'?>
<!-- Created with Cambalache 0.95.0 -->
<interface>
  <!-- interface-name venta_pago.ui -->
  <requires lib="gtk" version="4.0"/>
  <object class="GtkBox" id="box">
    <property name="margin-bottom">15</property>
    <property name="margin-end">15</property>
    <property name="margin-start">15</property>
    <property name="margin-top">15</property>
    <property name="orientation">vertical</property>
    <property name="spacing">20</property>
    <property name="vexpand">true</property>
    <child>
      <object class="GtkLabel" id="lbl_titulo">
        <property name="label">Venta / Cobro | Pago</property>
        <style>
          <class name="title-1"/>
        </style>
      </object>
    </child>
    <child>
      <object class="GtkListBox" id="list_venta">
        <child>
          <object class="GtkListBoxRow" id="BXRW1">
            <child>
              <object class="GtkBox" id="r1">
                <child>
                  <object class="GtkLabel">
                    <property name="label">Total</property>
                    <style>
                      <class name="title-3"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_monto_total">
                    <property name="halign">end</property>
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
        <child>
          <object class="GtkListBoxRow" id="BXRW2">
            <child>
              <object class="GtkBox" id="r2">
                <child>
                  <object class="GtkLabel">
                    <property name="label">Recibido</property>
                    <style>
                      <class name="title-3"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_recibido">
                    <property name="halign">end</property>
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
        <child>
          <object class="GtkListBoxRow" id="BXRW3">
            <child>
              <object class="GtkBox" id="r3">
                <child>
                  <object class="GtkLabel">
                    <property name="label">Falta</property>
                    <style>
                      <class name="title-3"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_faltante">
                    <property name="halign">end</property>
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
        <child>
          <object class="GtkListBoxRow" id="BXRW4">
            <child>
              <object class="GtkBox" id="r4">
                <child>
                  <object class="GtkLabel">
                    <property name="label">Cambio</property>
                    <style>
                      <class name="title-3"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_cambio">
                    <property name="halign">end</property>
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
        <style>
          <class name="rich-list"/>
          <class name="boxed-list"/>
        </style>
      </object>
    </child>
    <child>
      <object class="GtkLabel" id="lbl_mensaje_fin">
        <property name="visible">false</property>
        <style>
          <class name="title-2"/>
          <class name="dim-label"/>
        </style>
      </object>
    </child>
    <child>
      <object class="GtkLabel" id="lbl_timeout">
        <property name="label">Restante: 00:00</property>
      </object>
    </child>
    <child>
      <object class="GtkBox" id="box_action_timeout">
        <property name="homogeneous">true</property>
        <property name="orientation">vertical</property>
        <property name="spacing">10</property>
        <child>
          <object class="GtkButton" id="btn_timeout_cancel">
            <property name="label">Cancelar</property>
            <style>
              <class name="destructive-action"/>
            </style>
          </object>
        </child>
        <child>
          <object class="GtkButton" id="btn_timeout_retry">
            <property name="label">Necesito mas tiempo</property>
            <style>
              <class name="suggested-action"/>
            </style>
          </object>
        </child>
      </object>
    </child>
  </object>
</interface>)";
}
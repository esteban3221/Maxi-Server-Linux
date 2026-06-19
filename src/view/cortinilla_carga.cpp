#include "view/cortinilla_carga.hpp"

ViewCarga::ViewCarga(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : Gtk::Box(cobject)
{
  // Mapeo de widgets desde el Blueprint/XML
  v_img_main = refBuilder->get_widget<Gtk::Image>("img_main");
  v_lbl_main = refBuilder->get_widget<Gtk::Label>("lbl_main");
  v_lbl_sub = refBuilder->get_widget<Gtk::Label>("lbl_sub");
  v_spinner = refBuilder->get_widget<Gtk::Spinner>("spinner");
  v_btn_cancel_tarjeta = refBuilder->get_widget<Gtk::Button>("btn_cancel_tarjeta");
}

ViewCarga::~ViewCarga() {}

void ViewCarga::modo(bool is_efectivo)
{
  if (is_efectivo)
  {
    v_img_main->set_from_icon_name("usb-hub-symbolic");
    v_lbl_main->set_text("Espere un momento");
    v_lbl_sub->set_text("Cargando información de pago en efectivo...");
    v_btn_cancel_tarjeta->hide();
  }
  else
  {
    v_img_main->set_from_icon_name("phonelink-symbolic");
    v_lbl_main->set_text("Terminal de Pago Activa");
    v_lbl_sub->set_text("Por favor, siga las instrucciones en la pantalla de la terminal bancaria.");
    v_btn_cancel_tarjeta->show();
  }
}

namespace View
{
  const char *ui_cortinilla_carga = R"(
<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <requires lib="gtk" version="4.0"/>
  <object class="GtkBox" id="cortinilla_carga">
    <property name="orientation">1</property>
    <property name="valign">3</property>
    <property name="halign">3</property>
    <property name="spacing">32</property>
    <child>
      <object class="GtkImage" id="img_main">
        <property name="icon-name">credit-card-symbolic</property>
        <property name="pixel-size">128</property>
        <style>
          <class name="dim-label"/>
        </style>
      </object>
    </child>
    <child>
      <object class="GtkBox">
        <property name="orientation">1</property>
        <property name="spacing">8</property>
        <child>
          <object class="GtkLabel" id="lbl_main">
            <property name="label">Terminal de Pago Activa</property>
            <style>
              <class name="title-1"/>
            </style>
          </object>
        </child>
        <child>
          <object class="GtkLabel" id="lbl_sub">
            <property name="label">Por favor, siga las instrucciones en la pantalla de la terminal bancaria.</property>
            <property name="justify">2</property>
            <property name="wrap">true</property>
            <property name="max-width-chars">40</property>
            <style>
              <class name="body"/>
            </style>
          </object>
        </child>
      </object>
    </child>
    <child>
      <object class="GtkSpinner" id="spinner">
        <property name="spinning">true</property>
        <property name="width-request">48</property>
        <property name="height-request">48</property>
      </object>
    </child>
    <child>
      <object class="GtkButton" id="btn_cancel_tarjeta">
        <property name="label">Cancelar Transacción</property>
        <property name="halign">3</property>
        <property name="visible">false</property>
        <style>
          <class name="destructive-action"/>
          <class name="pill"/>
        </style>
        <property name="margin-top">20</property>
      </object>
    </child>
  </object>
</interface>
)";
}
#include "main_window.hpp"

VMainWindow::VMainWindow(/* args */) : ui(R"(
    <?xml version="1.0" encoding="UTF-8"?>
<interface>
  <requires lib="gtk" version="4.0"/>
  <object class="GtkBox" id="box_principal">
    <property name="orientation">1</property>
    <property name="vexpand">true</property>
    <property name="homogeneous">true</property>
    <property name="margin-bottom">10</property>
    <property name="margin-start">10</property>
    <property name="margin-end">10</property>
    <property name="margin-top">10</property>
    <child>
      <object class="GtkLabel" id="lbl_main">
        <property name="label">Bienvenido</property>
        <style>
          <class name="title-1"/>
        </style>
      </object>
    </child>
    <child>
      <object class="GtkOverlay">
        <property name="child">
          <object class="GtkImage" id="img_main_logo">
            <property name="file"></property>
            <property name="pixel-size">150</property>
            <property name="vexpand">true</property>
            <property name="use-fallback">true</property>
          </object>
        </property>
        <child type="overlay">
          <object class="GtkButton" id="btn_logo_nip">
            <property name="label">Maxicajero</property>
            <property name="halign">3</property>
            <property name="valign">3</property>
            <property name="margin-bottom">150</property>
            <property name="margin-start">150</property>
            <property name="margin-end">150</property>
            <property name="margin-top">150</property>
            <property name="opacity">0</property>
          </object>
        </child>
      </object>
    </child>
    <child>
      <object class="GtkButton" id="btn_pill">
        <property name="label" translatable="yes">1</property>
        <property name="halign">3</property>
        <property name="valign">3</property>
        <property name="vexpand">true</property>
        <property name="opacity">0</property>
        <style>
          <class name="pill"/>
          <class name="suggested-action"/>
        </style>
      </object>
    </child>
    <child>
      <object class="GtkLabel" id="lbl_version">
        <property name="label">Version ; 1.2.0.1</property>
        <property name="halign">1</property>
        <property name="valign">0</property>
      </object>
    </child>
  </object>
</interface>
    )")
{
    this->set_title("Maxicajero");

    Global::Widget::v_main_window = this;

    auto builder = Gtk::Builder::create_from_string(ui);

    this->v_box_principal = builder->get_widget<Gtk::Box>("box_principal");
    this->v_lbl_version = builder->get_widget<Gtk::Label>("lbl_version");
    this->v_img_main_logo = builder->get_widget<Gtk::Image>("img_main_logo");
    this->v_lbl_main = builder->get_widget<Gtk::Label>("lbl_main");
    this->v_btn_logo_nip = builder->get_widget<Gtk::Button>("btn_logo_nip");
    this->v_btn_pill = builder->get_widget<Gtk::Button>("btn_pill");

    v_lbl_version->set_label(Maxicajero::Version::getFullVersion());
    Global::Widget::v_main_stack = Gtk::manage(new Gtk::Stack);
    Global::Widget::v_main_stack->add(*v_box_principal, "0", "Home");
    Global::Widget::v_main_stack->set_interpolate_size(true);
    Global::Widget::v_main_stack->set_vhomogeneous(false);
    this->set_decorated(false);
    this->maximize();
    this->set_child(*Global::Widget::v_main_stack);
}

VMainWindow::~VMainWindow()
{
}
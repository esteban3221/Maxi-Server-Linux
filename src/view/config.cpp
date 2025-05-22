#include "view/config.hpp"

VConfig::VConfig(/* args */) : ui(R"(<?xml version='1.0' encoding='UTF-8'?>
<!-- Created with Cambalache 0.95.0 -->
<interface>
  <!-- interface-name home.ui -->
  <requires lib="gtk" version="4.12"/>
  <object class="GtkBox" id="left_box">
    <property name="orientation">vertical</property>
    <property name="spacing">5</property>
    <child>
      <object class="GtkButton" id="btn_back_config">
        <property name="child">
          <object class="GtkBox">
            <property name="spacing">5</property>
            <child>
              <object class="GtkImage">
                <property name="icon-name">go-previous-symbolic</property>
              </object>
            </child>
            <child>
              <object class="GtkLabel">
                <property name="label">Regresar</property>
              </object>
            </child>
          </object>
        </property>
        <property name="halign">fill</property>
        <property name="has-frame">false</property>
        <property name="margin-end">10</property>
        <property name="margin-start">10</property>
      </object>
    </child>
    <child>
      <object class="GtkSeparator"/>
    </child>
    <child>
      <object class="GtkStackSidebar" id="stackside">
        <property name="vexpand">true</property>
      </object>
    </child>
  </object>
</interface>)")
{
    auto builder = Gtk::Builder::create_from_string(ui);

    set_orientation(Gtk::Orientation::HORIZONTAL);
    v_btn_back_config = builder->get_widget<Gtk::Button>("btn_back_config");
    v_left_box = builder->get_widget<Gtk::Box>("left_box");
    v_stack_side = builder->get_widget<Gtk::StackSidebar>("stackside");

    append(*v_left_box);

    v_stack_side->set_stack(v_stack);
    v_stack.set_margin(10);
    v_stack.set_hexpand();
    v_stack.set_transition_type(Gtk::StackTransitionType::SLIDE_UP_DOWN);

    //llenado de Vistas de Configuracion
    builder = Gtk::Builder::create_from_string(View::ui_general);
    auto general = Gtk::Builder::get_widget_derived<General>(builder, "page_0");
    v_stack.add(*general,"0","General");

    builder = Gtk::Builder::create_from_string(View::ui_info);
    auto info = Gtk::Builder::get_widget_derived<Info>(builder, "page_1");
    v_stack.add(*info,"1","Informacion");

    builder = Gtk::Builder::create_from_string(View::ui_empresa);
    auto empresa = Gtk::Builder::get_widget_derived<Empresa>(builder, "page_2");
    v_stack.add(*empresa,"2","Empresa");

    builder = Gtk::Builder::create_from_string(View::ui_impresora);
    auto impresora = Gtk::Builder::get_widget_derived<Impresora>(builder, "page_3");
    v_stack.add(*impresora,"3","Impresora");

    builder = Gtk::Builder::create_from_string(View::ui_roles);
    auto roles = Gtk::Builder::get_widget_derived<Roles>(builder, "page_4");
    v_stack.add(*roles,"4","Roles");

    builder = Gtk::Builder::create_from_string(View::ui_wifi);
    auto wifi = Gtk::Builder::get_widget_derived<Wifi>(builder, "page_5");
    v_stack.add(*wifi,"5","Wifi");

    auto detalles_validadores = Gtk::manage(new DetallesValidador);
    v_stack.add(*detalles_validadores,"6","Detalle Validadores");
    v_stack.set_interpolate_size(true);

    append(v_stack);
}

VConfig::~VConfig()
{
}
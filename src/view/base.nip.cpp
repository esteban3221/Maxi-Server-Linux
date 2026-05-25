#include "view/base.nip.hpp"

VBaseNip::VBaseNip(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                            Gtk::Box(cobject)
{
    this->v_btn_nip_back = m_builder->get_widget<Gtk::Button>("btn_nip_back");
    this->v_ety_pin = m_builder->get_widget<Gtk::PasswordEntry>("ety_pin");
    this->v_spn_monto = m_builder->get_widget<Gtk::SpinButton>("spn_monto");

    v_active_editable = v_ety_pin;

    v_btn_nip_back->signal_clicked().connect(sigc::mem_fun(*this, &VBaseNip::on_btn_nip_back));
    ((Gtk::Entry*)v_ety_pin)->signal_activate().connect(sigc::mem_fun(*this, &VBaseNip::on_btn_nip_enter));
    //((Gtk::Entry*)v_ety_pin)->set_input_purpose(Gtk::InputPurpose::PIN);
}

void VBaseNip::set_modo_monto(bool es_monto) 
{
    v_ety_pin->set_visible(!es_monto);
    v_spn_monto->set_visible(es_monto);

    v_spn_monto->set_digits(2);
    
    if(es_monto) v_active_editable = dynamic_cast<Gtk::Editable*>(v_spn_monto);
    else v_active_editable = dynamic_cast<Gtk::Editable*>(v_ety_pin);
}

VBaseNip::~VBaseNip()
{
}

namespace View
{
    const char *ui_nip = R"(
<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <requires lib="gtk" version="4.0"/>
  <object class="GtkBox" id="box_nip">
    <property name="orientation">1</property>
    <property name="spacing">10</property>
    <property name="margin-bottom">10</property>
    <property name="margin-start">10</property>
    <property name="margin-end">10</property>
    <property name="margin-top">10</property>
    <child>
      <object class="GtkBox">
        <property name="spacing">10</property>
        <property name="orientation">0</property>
        <child>
          <object class="GtkButton" id="btn_nip_back">
            <property name="icon-name">go-previous-symbolic</property>
            <property name="valign">1</property>
            <style>
              <class name="osd"/>
            </style>
          </object>
        </child>
        <child>
          <object class="GtkBox">
            <property name="orientation">1</property>
            <property name="halign">0</property>
            <property name="hexpand">true</property>
            <child>
              <object class="GtkPasswordEntry" id="ety_pin">
                <property name="placeholder-text">Digite NIP</property>
                <property name="height-request">100</property>
                <property name="hexpand">true</property>
                <property name="xalign">0.5</property>
                <style>
                  <class name="title-2"/>
                </style>
              </object>
            </child>
            <child>
              <object class="GtkSpinButton" id="spn_monto">
                <property name="visible">false</property>
                <property name="can-focus">true</property>
                <property name="digits">2</property>
                <property name="numeric">true</property>
                <property name="height-request">100</property>
                <property name="xalign">0.5</property>
                <property name="adjustment">
                  <object class="GtkAdjustment">
                    <property name="lower">0</property>
                    <property name="upper">999999</property>
                    <property name="step-increment">0.01</property>
                  </object>
                </property>
                <style>
                  <class name="title-1"/>
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
#include "view/refill.hpp"

VRefill::VRefill(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : Gtk::Box(cobject)
{
    Global::Widget::Refill::v_tree_reciclador_monedas  = refBuilder->get_widget<Gtk::ColumnView>("treeRecicladorMonedas");
    Global::Widget::Refill::v_tree_reciclador_billetes = refBuilder->get_widget<Gtk::ColumnView>("treeRecicladorBilletes");

    v_lbl_total_parcial_monedas = refBuilder->get_widget<Gtk::Label>("lblTotalMXN1");
    v_lbl_total_parcial_billetes = refBuilder->get_widget<Gtk::Label>("lblTotalMXN2");
    v_lbl_total_monedas = refBuilder->get_widget<Gtk::Label>("lblTotalMXN4");
    v_lbl_total_billetes = refBuilder->get_widget<Gtk::Label>("lblTotalMXN5");
    v_lbl_total = refBuilder->get_widget<Gtk::Label>("lblTotalMXN3");

    v_btn_deten = refBuilder->get_widget<Gtk::Button>("btn_detener");
    v_btn_deten->set_sensitive(false);
}

void VRefill::on_setup_label(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    list_item->set_child(*Gtk::make_managed<Gtk::Label>("", Gtk::Align::END));
}

void VRefill::on_bind_deno(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MLevelCash>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_denominacion));
}

void VRefill::on_bind_alm(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MLevelCash>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_cant_alm));
}

void VRefill::on_bind_recy(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MLevelCash>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_cant_recy));
}

void VRefill::on_bind_ingreso(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MLevelCash>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_ingreso));
}

void VRefill::on_bind_inmo(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MLevelCash>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_nivel_inmo));
}


VRefill::~VRefill()
{
}

namespace View
{
    const char *ui_refill = R"(<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <requires lib="gtk" version="4.0"/>
  <object class="GtkBox" id="boxVistaEfectivo">
    <property name="halign">0</property>
    <property name="margin-bottom">15</property>
    <property name="margin-end">15</property>
    <property name="margin-start">15</property>
    <property name="margin-top">15</property>
    <property name="spacing">20</property>
    <child>
      <object class="GtkBox">
        <property name="homogeneous">true</property>
        <property name="orientation">1</property>
        <property name="spacing">20</property>
        <property name="vexpand">true</property>
        <child>
          <object class="GtkFrame" id="frmRecicladorMonedas">
            <child>
              <object class="GtkBox">
                <property name="margin-end">12</property>
                <property name="margin-start">12</property>
                <property name="orientation">1</property>
                <child>
                  <object class="GtkLabel">
                    <property name="label">Monedas</property>
                    <style>
                      <class name="title-1"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkBox">
                    <property name="margin-bottom">12</property>
                    <property name="margin-top">12</property>
                    <property name="spacing">10</property>
                    <property name="vexpand">true</property>
                    <child>
                      <object class="GtkColumnView" id="treeRecicladorMonedas">
                        <property name="enable-rubberband">true</property>
                        <property name="hexpand">true</property>
                        <property name="hexpand-set">true</property>
                        <property name="show-column-separators">true</property>
                        <property name="vexpand">true</property>
                        <property name="vexpand-set">true</property>
                      </object>
                    </child>
                    <child>
                      <object class="GtkBox">
                        <property name="halign">2</property>
                        <property name="hexpand-set">true</property>
                        <property name="orientation">1</property>
                        <child>
                          <object class="GtkLabel">
                            <property name="label">Total</property>
                            <style>
                              <class name="title-2"/>
                            </style>
                          </object>
                        </child>
                        <child>
                          <object class="GtkLabel" id="lblTotalMXN1">
                            <property name="label">0.00 MXN</property>
                            <style>
                              <class name="title-4"/>
                            </style>
                          </object>
                        </child>
                      </object>
                    </child>
                  </object>
                </child>
              </object>
            </child>
          </object>
        </child>
        <child>
          <object class="GtkFrame" id="frmRecicladorBilletes">
            <child>
              <object class="GtkBox">
                <property name="margin-end">12</property>
                <property name="margin-start">12</property>
                <property name="orientation">1</property>
                <child>
                  <object class="GtkLabel">
                    <property name="label">Billetes</property>
                    <style>
                      <class name="title-1"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkBox">
                    <property name="margin-bottom">12</property>
                    <property name="margin-top">12</property>
                    <property name="spacing">10</property>
                    <child>
                      <object class="GtkColumnView" id="treeRecicladorBilletes">
                        <property name="enable-rubberband">true</property>
                        <property name="hexpand">true</property>
                        <property name="hexpand-set">true</property>
                        <property name="show-column-separators">true</property>
                        <property name="vexpand">true</property>
                        <property name="vexpand-set">true</property>
                      </object>
                    </child>
                    <child>
                      <object class="GtkBox">
                        <property name="halign">2</property>
                        <property name="orientation">1</property>
                        <child>
                          <object class="GtkLabel">
                            <property name="label">Total</property>
                            <style>
                              <class name="title-2"/>
                            </style>
                          </object>
                        </child>
                        <child>
                          <object class="GtkLabel" id="lblTotalMXN2">
                            <property name="label">0.00 MXN</property>
                            <style>
                              <class name="title-4"/>
                            </style>
                          </object>
                        </child>
                      </object>
                    </child>
                  </object>
                </child>
              </object>
            </child>
          </object>
        </child>
      </object>
    </child>
    <child>
      <object class="GtkBox">
        <property name="orientation">1</property>
        <property name="spacing">20</property>
        <child>
          <object class="GtkFrame" id="frmTotal">
            <child>
              <object class="GtkBox">
                <property name="margin-bottom">12</property>
                <property name="margin-end">12</property>
                <property name="margin-start">12</property>
                <property name="margin-top">12</property>
                <property name="orientation">1</property>
                <child>
                  <object class="GtkLabel">
                    <property name="label">Total</property>
                    <style>
                      <class name="title-1"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lblTotalMXN3">
                    <property name="label">0.00 MXN</property>
                    <style>
                      <class name="title-2"/>
                    </style>
                  </object>
                </child>
              </object>
            </child>
          </object>
        </child>
        <child>
          <object class="GtkFrame" id="frmTotal2">
            <child>
              <object class="GtkBox">
                <property name="margin-bottom">12</property>
                <property name="margin-end">12</property>
                <property name="margin-start">12</property>
                <property name="margin-top">12</property>
                <property name="orientation">1</property>
                <child>
                  <object class="GtkLabel">
                    <property name="label">Total Monedas</property>
                    <style>
                      <class name="title-1"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lblTotalMXN4">
                    <property name="label">0.00 MXN</property>
                    <style>
                      <class name="title-2"/>
                    </style>
                  </object>
                </child>
              </object>
            </child>
          </object>
        </child>
        <child>
          <object class="GtkFrame" id="frmTotal3">
            <child>
              <object class="GtkBox">
                <property name="margin-bottom">12</property>
                <property name="margin-end">12</property>
                <property name="margin-start">12</property>
                <property name="margin-top">12</property>
                <property name="orientation">1</property>
                <child>
                  <object class="GtkLabel">
                    <property name="label">Total Billetes</property>
                    <style>
                      <class name="title-1"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lblTotalMXN5">
                    <property name="label">0.00 MXN</property>
                    <style>
                      <class name="title-2"/>
                    </style>
                  </object>
                </child>
              </object>
            </child>
          </object>
        </child>
        <child>
          <object class="GtkButton" id="btn_detener">
            <property name="label" translatable="yes">Detener</property>
            <property name="css-classes">destructive-action</property>
          </object>
        </child>
      </object>
    </child>
  </object>
</interface>)";
}
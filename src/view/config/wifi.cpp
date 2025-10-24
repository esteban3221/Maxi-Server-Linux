#include "view/config/wifi.hpp"

VWifi::VWifi(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                      Gtk::Box(cobject)
{
    this->v_lbl_red[0] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_ip_wifi");
    this->v_lbl_red[1] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_mac_wifi");
    this->v_lbl_red[2] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_ip_eth");
    this->v_lbl_red[3] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_ip_eth_mac");
    this->v_lbl_red[4] = this->m_builder->get_widget<Gtk::EditableLabel>("ety_info_ssid_wifi");

    this->v_btn_red = this->m_builder->get_widget<Gtk::Button>("btn_red");
    this->v_btn_redes = this->m_builder->get_widget<Gtk::Button>("btn_redes");
    this->v_btn_regresar = this->m_builder->get_widget<Gtk::Button>("btn_regresar");
    this->v_btn_refresh = this->m_builder->get_widget<Gtk::Button>("btn_refresh");
    this->v_stack_wifi = this->m_builder->get_widget<Gtk::Stack>("stack_wifi");
    //this->v_list_info_red = this->m_builder->get_widget<Gtk::ListBox>("list_info_red");
    this->v_list_box_wifi = this->m_builder->get_widget<Gtk::ListBox>("list_box_wifi");
}

VWifi::~VWifi()
{
}

VWifi::VWifiRow::VWifiRow(const std::string &titulo,const std::string &subtitulo)
{
    v_image_wifi = Gtk::manage(new Gtk::Image());
    v_image_wifi->set_from_icon_name("network-wireless-symbolic");
    v_image_wifi->set_icon_size(Gtk::IconSize::LARGE);

    v_image_check = Gtk::manage(new Gtk::Image());
    v_image_check->set_from_icon_name("preferences-system-network-symbolic");

    v_SSID = Gtk::manage(new Gtk::Label(titulo));
    v_SSID->set_halign(Gtk::Align::START);
    v_SSID->set_css_classes({"title-4"});
    v_subtitulo = Gtk::manage(new Gtk::Label(subtitulo));
    v_subtitulo->set_halign(Gtk::Align::START);
    v_subtitulo->set_css_classes({"dim-label"});
    v_subtitulo->set_ellipsize(Pango::EllipsizeMode::END);
    v_subtitulo->set_justify(Gtk::Justification::LEFT);

    auto box_label = Gtk::manage(new Gtk::Box(Gtk::Orientation::VERTICAL));
    box_label->append(*v_SSID);
    box_label->append(*v_subtitulo);
    box_label->set_hexpand();

    v_box = Gtk::manage(new Gtk::Box(Gtk::Orientation::HORIZONTAL));
    v_box->append(*v_image_wifi);
    v_box->append(*box_label);
    v_box->append(*v_image_check);
    v_image_check->set_halign(Gtk::Align::END);
    v_box->set_margin(5);

    set_child(*v_box);
}

namespace View
{
    const char *ui_wifi = R"(<?xml version='1.0' encoding='UTF-8'?>
<!-- Created with Cambalache 0.96.1 -->
<interface>
  <!-- interface-name wifi.ui -->
  <requires lib="gtk" version="4.0"/>
  <object class="GtkBox" id="page_5">
    <property name="orientation">vertical</property>
    <property name="spacing">10</property>
    <child>
      <object class="GtkStack" id="stack_wifi">
        <property name="transition-type">over-left-right</property>
        <child>
          <object class="GtkStackPage">
            <property name="child">
              <object class="GtkBox">
                <property name="orientation">vertical</property>
                <child>
                  <object class="GtkLabel">
                    <property name="label">Inalámbrica</property>
                    <property name="margin-bottom">10</property>
                    <property name="xalign">0</property>
                    <style>
                      <class name="title-2"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkListBox" id="list_info_red">
                    <child>
                      <object class="GtkListBoxRow">
                        <property name="activatable">false</property>
                        <child>
                          <object class="GtkBox">
                            <child>
                              <object class="GtkLabel">
                                <property name="halign">start</property>
                                <property name="label">Red</property>
                              </object>
                            </child>
                            <child>
                              <object class="GtkEditableLabel" id="ety_info_ssid_wifi">
                                <property name="editable">false</property>
                                <property name="halign">end</property>
                                <property name="hexpand">true</property>
                                <property name="text">Type here…</property>
                                <style>
                                  <class name="dim-label"/>
                                </style>
                              </object>
                            </child>
                          </object>
                        </child>
                      </object>
                    </child>
                    <child>
                      <object class="GtkListBoxRow">
                        <property name="activatable">false</property>
                        <child>
                          <object class="GtkBox">
                            <child>
                              <object class="GtkLabel">
                                <property name="halign">start</property>
                                <property name="label">Direccion IP</property>
                              </object>
                            </child>
                            <child>
                              <object class="GtkEditableLabel" id="ety_info_ip_wifi">
                                <property name="editable">false</property>
                                <property name="halign">end</property>
                                <property name="hexpand">true</property>
                                <property name="text">Type here…</property>
                                <style>
                                  <class name="dim-label"/>
                                </style>
                              </object>
                            </child>
                          </object>
                        </child>
                      </object>
                    </child>
                    <child>
                      <object class="GtkListBoxRow">
                        <property name="activatable">false</property>
                        <child>
                          <object class="GtkBox">
                            <child>
                              <object class="GtkLabel">
                                <property name="halign">start</property>
                                <property name="label">Direccion Mac</property>
                              </object>
                            </child>
                            <child>
                              <object class="GtkEditableLabel" id="ety_info_mac_wifi">
                                <property name="editable">false</property>
                                <property name="halign">end</property>
                                <property name="hexpand">true</property>
                                <property name="text">Type here…</property>
                                <style>
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
                  <object class="GtkButton" id="btn_redes">
                    <property name="halign">center</property>
                    <property name="label">Conectar Wi-Fi</property>
                    <property name="margin-top">40</property>
                    <property name="valign">center</property>
                    <style>
                      <class name="suggested-action"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkBox">
                    <property name="orientation">vertical</property>
                    <child>
                      <object class="GtkLabel">
                        <property name="label">Ethernet</property>
                        <property name="margin-bottom">20</property>
                        <property name="margin-top">20</property>
                        <property name="xalign">0</property>
                        <style>
                          <class name="title-2"/>
                        </style>
                      </object>
                    </child>
                    <child>
                      <object class="GtkListBox" id="list_info_con">
                        <child>
                          <object class="GtkListBoxRow">
                            <property name="activatable">false</property>
                            <child>
                              <object class="GtkBox">
                                <child>
                                  <object class="GtkLabel">
                                    <property name="halign">start</property>
                                    <property name="label">Direccion IP</property>
                                  </object>
                                </child>
                                <child>
                                  <object class="GtkEditableLabel" id="ety_info_ip_eth">
                                    <property name="editable">false</property>
                                    <property name="halign">end</property>
                                    <property name="hexpand">true</property>
                                    <property name="text">Type here…</property>
                                    <style>
                                      <class name="dim-label"/>
                                    </style>
                                  </object>
                                </child>
                              </object>
                            </child>
                          </object>
                        </child>
                        <child>
                          <object class="GtkListBoxRow">
                            <property name="activatable">false</property>
                            <child>
                              <object class="GtkBox">
                                <child>
                                  <object class="GtkLabel">
                                    <property name="halign">start</property>
                                    <property name="label">Direccion MAC</property>
                                  </object>
                                </child>
                                <child>
                                  <object class="GtkEditableLabel" id="ety_info_ip_eth_mac">
                                    <property name="editable">false</property>
                                    <property name="halign">end</property>
                                    <property name="hexpand">true</property>
                                    <property name="text">Type here…</property>
                                    <style>
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
                      <object class="GtkButton" id="btn_red">
                        <property name="halign">center</property>
                        <property name="label">Gestionar conexiones</property>
                        <property name="margin-top">40</property>
                        <property name="valign">center</property>
                        <style>
                          <class name="warning"/>
                        </style>
                      </object>
                    </child>
                  </object>
                </child>
              </object>
            </property>
            <property name="name">status_red</property>
          </object>
        </child>
        <child>
          <object class="GtkStackPage">
            <property name="child">
              <object class="GtkViewport">
                <property name="child">
                  <object class="GtkBox">
                    <property name="orientation">vertical</property>
                    <child>
                      <object class="GtkBox">
                        <property name="hexpand">True</property>
                        <child>
                          <object class="GtkButton" id="btn_regresar">
                            <property name="icon-name">back</property>
                            <style>
                              <class name="opaque"/>
                            </style>
                          </object>
                        </child>
                        <child>
                          <object class="GtkLabel">
                            <property name="halign">baseline-center</property>
                            <property name="hexpand">True</property>
                            <property name="label">Redes disponibles</property>
                            <property name="margin-bottom">10</property>
                            <property name="xalign">0</property>
                            <style>
                              <class name="title-2"/>
                            </style>
                          </object>
                        </child>
                        <child>
                          <object class="GtkButton" id="btn_refresh">
                            <property name="icon-name">object-rotate-left-symbolic</property>
                          </object>
                        </child>
                      </object>
                    </child>
                    <child>
                      <object class="GtkScrolledWindow">
                        <property name="margin-bottom">20</property>
                        <property name="margin-end">40</property>
                        <property name="margin-start">40</property>
                        <property name="margin-top">20</property>
                        <property name="vexpand">True</property>
                        <child>
                          <object class="GtkListBox" id="list_box_wifi">
                            <style>
                              <class name="rich-list"/>
                              <class name="boxed-list"/>
                            </style>
                          </object>
                        </child>
                      </object>
                    </child>
                  </object>
                </property>
              </object>
            </property>
            <property name="name">redes_wifi</property>
          </object>
        </child>
      </object>
    </child>
  </object>
</interface>
)";
}
#include "view/config/validador.hpp"

VDetallesValidador::VDetallesValidador(/* args */) : v_box_header(Gtk::Orientation::HORIZONTAL),
                                                     v_box_contenedor(Gtk::Orientation::HORIZONTAL),
                                                     ui(R"(<?xml version='1.0' encoding='UTF-8'?>
<!-- Created with Cambalache 0.96.0 -->
<interface>
  <!-- interface-name validadores.ui -->
  <requires lib="gtk" version="4.12"/>
  <object class="GtkBox" id="box_detalles_validador">
    <property name="margin-bottom">10</property>
    <property name="margin-end">10</property>
    <property name="margin-start">10</property>
    <property name="margin-top">10</property>
    <property name="orientation">vertical</property>
    <property name="spacing">10</property>
    <child>
      <object class="GtkListBox" id="list_configurable">
        <property name="activate-on-single-click">False</property>
        <property name="selection-mode">none</property>
        <child>
          <object class="GtkListBoxRow">
            <property name="child">
              <object class="GtkLabel" id="lbl_titulo">
                <property name="label">Sin Definir</property>
                <style>
                  <class name="title-3"/>
                </style>
              </object>
            </property>
          </object>
        </child>
        <child>
          <object class="GtkListBoxRow">
            <property name="child">
              <object class="GtkImage" id="img_validador">
                <property name="icon-name">dialog-question-symbolic</property>
                <property name="margin-bottom">10</property>
                <property name="margin-end">10</property>
                <property name="margin-start">10</property>
                <property name="margin-top">10</property>
                <property name="pixel-size">250</property>
              </object>
            </property>
          </object>
        </child>
        <style>
          <class name="rich-list"/>
          <class name="boxed-list"/>
        </style>
      </object>
    </child>
    <child>
      <object class="GtkListBox" id="list_detalles">
        <property name="activate-on-single-click">False</property>
        <property name="selection-mode">none</property>
        <child>
          <object class="GtkListBoxRow">
            <child>
              <object class="GtkBox">
                <child>
                  <object class="GtkLabel">
                    <property name="halign">start</property>
                    <property name="hexpand">true</property>
                    <property name="label">Puerto</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
                    <style>
                      <class name="title-5"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkDropDown" id="drop_puerto"/>
                </child>
              </object>
            </child>
          </object>
        </child>
        <child>
          <object class="GtkListBoxRow">
            <child>
              <object class="GtkBox">
                <child>
                  <object class="GtkLabel">
                    <property name="halign">start</property>
                    <property name="hexpand">true</property>
                    <property name="label">ID</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
                    <style>
                      <class name="title-5"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_data_0">
                    <property name="halign">end</property>
                    <property name="hexpand">true</property>
                    <property name="label">--</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
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
            <child>
              <object class="GtkBox">
                <child>
                  <object class="GtkLabel">
                    <property name="halign">start</property>
                    <property name="hexpand">true</property>
                    <property name="label">Conectado</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
                    <style>
                      <class name="title-5"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_data_1">
                    <property name="halign">end</property>
                    <property name="hexpand">true</property>
                    <property name="label">--</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
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
            <child>
              <object class="GtkBox">
                <child>
                  <object class="GtkLabel">
                    <property name="halign">start</property>
                    <property name="hexpand">true</property>
                    <property name="label">Ver. SSP</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
                    <style>
                      <class name="title-5"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_data_2">
                    <property name="halign">end</property>
                    <property name="hexpand">true</property>
                    <property name="label">--</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
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
            <child>
              <object class="GtkBox">
                <child>
                  <object class="GtkLabel">
                    <property name="halign">start</property>
                    <property name="hexpand">true</property>
                    <property name="label">Error</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
                    <style>
                      <class name="title-5"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_data_3">
                    <property name="halign">end</property>
                    <property name="hexpand">true</property>
                    <property name="label">--</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
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
            <child>
              <object class="GtkBox">
                <child>
                  <object class="GtkLabel">
                    <property name="halign">start</property>
                    <property name="hexpand">true</property>
                    <property name="label">Firmware</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
                    <style>
                      <class name="title-5"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_data_4">
                    <property name="halign">end</property>
                    <property name="hexpand">true</property>
                    <property name="label">--</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
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
            <child>
              <object class="GtkBox">
                <child>
                  <object class="GtkLabel">
                    <property name="halign">start</property>
                    <property name="hexpand">true</property>
                    <property name="label">Set Regional</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
                    <style>
                      <class name="title-5"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_data_5">
                    <property name="halign">end</property>
                    <property name="hexpand">true</property>
                    <property name="label">--</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
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
            <child>
              <object class="GtkBox">
                <child>
                  <object class="GtkLabel">
                    <property name="halign">start</property>
                    <property name="hexpand">true</property>
                    <property name="label">ID Serial</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
                    <style>
                      <class name="title-5"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_data_6">
                    <property name="halign">end</property>
                    <property name="hexpand">true</property>
                    <property name="label">--</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
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
            <child>
              <object class="GtkBox">
                <child>
                  <object class="GtkLabel">
                    <property name="halign">start</property>
                    <property name="hexpand">true</property>
                    <property name="label">ID Modulo</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
                    <style>
                      <class name="title-5"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_data_7">
                    <property name="halign">end</property>
                    <property name="hexpand">true</property>
                    <property name="label">--</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
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
            <child>
              <object class="GtkBox">
                <child>
                  <object class="GtkLabel">
                    <property name="halign">start</property>
                    <property name="hexpand">true</property>
                    <property name="label">Revision</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
                    <style>
                      <class name="title-5"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_data_8">
                    <property name="halign">end</property>
                    <property name="hexpand">true</property>
                    <property name="label">--</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
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
            <child>
              <object class="GtkBox">
                <child>
                  <object class="GtkLabel">
                    <property name="halign">start</property>
                    <property name="hexpand">true</property>
                    <property name="label">Revision Modulo</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
                    <style>
                      <class name="title-5"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_data_9">
                    <property name="halign">end</property>
                    <property name="hexpand">true</property>
                    <property name="label">--</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
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
            <child>
              <object class="GtkBox">
                <child>
                  <object class="GtkLabel">
                    <property name="halign">start</property>
                    <property name="hexpand">true</property>
                    <property name="label">Reloj Interno</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
                    <style>
                      <class name="title-5"/>
                    </style>
                  </object>
                </child>
                <child>
                  <object class="GtkLabel" id="lbl_data_10">
                    <property name="ellipsize">start</property>
                    <property name="halign">end</property>
                    <property name="hexpand">true</property>
                    <property name="label">--</property>
                    <property name="valign">center</property>
                    <property name="xalign">0</property>
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
  </object>
</interface>)")
{
    v_box.set_spacing(5);

    auto builder = Gtk::Builder::create_from_string(ui);

    v_box_header.set_halign(Gtk::Align::FILL);
    v_box_header.set_hexpand();

    v_box_header.append(v_lbl_header);
    v_box_header.append(v_btn_test_coneccion);

    v_lbl_header.set_text("Detalle Validadores");
    v_lbl_header.add_css_class("title-2");
    v_lbl_header.set_hexpand();
    v_lbl_header.set_halign(Gtk::Align::START);

    v_btn_test_coneccion.set_icon_name("am-network-symbolic");
    v_btn_test_coneccion.set_tooltip_text("Cambia puerto de coneccion de los validadores");
    v_btn_test_coneccion.set_halign(Gtk::Align::END);

    v_lbl_subtitulo.set_text("Visualiza datos internos para monitorear estatus de los validadores y/o modifica el puerto serial de conección.");
    v_lbl_subtitulo.add_css_class("dim-label");
    v_lbl_subtitulo.set_margin_top(10);
    v_lbl_subtitulo.set_margin_bottom(10);

    v_box.append(v_box_header);
    v_box.append(v_lbl_subtitulo);

    auto path = Glib::get_user_special_dir(Glib::UserDirectory::DOCUMENTS);

    v_box_bill = Gtk::Builder::get_widget_derived<VValidador>(builder, "box_detalles_validador");
    v_box_bill->set_id_conf(1);
    v_box_bill->set_img(path + "/img/validadores/SPECTRAL_PAYOUT.png");


    builder = Gtk::Builder::create_from_string(ui);
    v_box_coin = Gtk::Builder::get_widget_derived<VValidador>(builder, "box_detalles_validador");
    v_box_coin->set_id_conf(3);
    v_box_coin->set_img(path + "/img/validadores/scs.png");

    v_box_contenedor.append(*v_box_bill);
    v_box_contenedor.append(*v_box_coin);
    v_box_contenedor.set_homogeneous();

    v_box.append(v_box_contenedor);
    v_box.set_orientation(Gtk::Orientation::VERTICAL);

    this->set_child(v_box);
}

VDetallesValidador::~VDetallesValidador()
{
}
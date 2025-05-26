#include "main_window.hpp"

VMainWindow::VMainWindow(/* args */) : ui(R"(<interface>
    <requires lib="gtk" version="4.0"/>
    <object class="GtkBox" id="box_principal">
        <property name="orientation">1</property>
        <property name="homogeneous">false</property>
        <property name="margin-bottom">10</property>
        <property name="margin-start">10</property>
        <property name="margin-end">10</property>
        <property name="margin-top">10</property>
        <child>
            <object class="GtkBox">
                <property name="homogeneous">true</property>
                <property name="orientation">1</property>
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
                                <property name="file">/home/esteban/Descargas/ll/TicketImages/logo_blanco.png</property>
                                <property name="pixel-size">150</property>
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
            </object>
        </child>
        <child>
            <object class="GtkButton" id="btn_pill">
                <property name="label" translatable="true">1</property>
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
                <property name="label">Version ; 1.0.0.1</property>
                <property name="halign">2</property>
                <property name="valign">2</property>
            </object>
        </child>
    </object>
</interface>)")
{
    this->set_title("MaxiCajero");
    this->set_default_size(800, 600);

    Global::Widget::v_main_window = this;

    auto builder = Gtk::Builder::create_from_string(ui);

    this->v_box_principal = builder->get_widget<Gtk::Box>("box_principal");
    this->v_lbl_version = builder->get_widget<Gtk::Label>("lbl_version");
    this->v_img_main_logo = builder->get_widget<Gtk::Image>("img_main_logo");
    this->v_lbl_main = builder->get_widget<Gtk::Label>("lbl_main");
    this->v_btn_logo_nip = builder->get_widget<Gtk::Button>("btn_logo_nip");
    this->v_btn_pill = builder->get_widget<Gtk::Button>("btn_pill");

    Global::Widget::v_main_stack = Gtk::manage(new Gtk::Stack);
    Global::Widget::v_main_stack->add(*v_box_principal, "0", "Home");

    // widgets dervados
    builder = Gtk::Builder::create_from_string(View::ui_vp);
    auto pago = Gtk::Builder::get_widget_derived<Pago>(builder, "box");

    builder = Gtk::Builder::create_from_string(View::ui_vp);
    auto venta = Gtk::Builder::get_widget_derived<Venta>(builder, "box");

    builder = Gtk::Builder::create_from_string(View::ui_vp);
    auto pago_manual = Gtk::Builder::get_widget_derived<PagoManual>(builder, "box");

    Global::Widget::v_main_stack->add(*pago, "1", "Pago");
    Global::Widget::v_main_stack->add(*pago_manual, "2", "Pago Manual");
    Global::Widget::v_main_stack->add(*venta, "3", "Venta");

    builder = Gtk::Builder::create_from_string(View::ui_refill);
    auto refill = Gtk::Builder::get_widget_derived<Refill>(builder, "boxVistaEfectivo");

    Global::Widget::v_main_stack->add(*refill, "4", "Refill");

    auto config = Gtk::manage(new Config); 
    Global::Widget::v_main_stack->add(*config, "5", "Configuracion");

    builder = Gtk::Builder::create_from_string(View::ui_nip);
    auto nip = Gtk::Builder::get_widget_derived<Nip>(builder, "box_nip");
    Global::Widget::v_main_stack->add(*nip, "6", "Nip");

    builder = Gtk::Builder::create_from_string(View::ui_nip);
    auto nnip = Gtk::Builder::get_widget_derived<NuevoNip>(builder, "box_nip");
    Global::Widget::v_main_stack->add(*nnip, "7", "Nuevo Nip");

    Global::Widget::v_main_stack->set_transition_type(Gtk::StackTransitionType::ROTATE_LEFT_RIGHT);
    this->set_decorated(false);
    this->maximize();

    this->set_child(*Global::Widget::v_main_stack);
}

VMainWindow::~VMainWindow()
{
}
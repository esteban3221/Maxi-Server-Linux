#include "main_window.hpp"

VMainWindow::VMainWindow(/* args */)
{
    this->set_title("MaxiCajero");
    this->set_default_size(800, 600);

    Global::Widget::v_main_window = this;

    auto builder = Gtk::Builder::create_from_file("../test/ui/main_window.ui");

    this->v_box_principal = builder->get_widget<Gtk::Box>("box_principal");
    this->v_lbl_version = builder->get_widget<Gtk::Label>("lbl_version");
    this->v_img_main_logo = builder->get_widget<Gtk::Image>("img_main_logo");
    this->v_lbl_main = builder->get_widget<Gtk::Label>("lbl_main");
    this->v_btn_logo_nip = builder->get_widget<Gtk::Button>("btn_logo_nip");
    this->v_btn_pill = builder->get_widget<Gtk::Button>("btn_pill");

    Global::Widget::v_main_stack = Gtk::manage(new Gtk::Stack);
    Global::Widget::v_main_stack->add(*v_box_principal, "0", "Home");

    // widgets dervados
    builder = Gtk::Builder::create_from_file("../test/ui/base/venta_pago.ui");
    auto pago = Gtk::Builder::get_widget_derived<Pago>(builder, "box");

    builder = Gtk::Builder::create_from_file("../test/ui/base/venta_pago.ui");
    auto venta = Gtk::Builder::get_widget_derived<Venta>(builder, "box");

    builder = Gtk::Builder::create_from_file("../test/ui/base/venta_pago.ui");
    auto pago_manual = Gtk::Builder::get_widget_derived<PagoManual>(builder, "box");

    Global::Widget::v_main_stack->add(*pago, "1", "Pago");
    Global::Widget::v_main_stack->add(*pago_manual, "2", "Pago Manual");
    Global::Widget::v_main_stack->add(*venta, "3", "Venta");

    builder = Gtk::Builder::create_from_file("../test/ui/refill.ui");
    auto refill = Gtk::Builder::get_widget_derived<Refill>(builder, "boxVistaEfectivo");

    Global::Widget::v_main_stack->add(*refill, "4", "Refill");

    auto config = Gtk::manage(new Config); 
    Global::Widget::v_main_stack->add(*config, "5", "Configuracion");

    builder = Gtk::Builder::create_from_file("../test/ui/nip.ui");
    auto nip = Gtk::Builder::get_widget_derived<Nip>(builder, "box_nip");
    Global::Widget::v_main_stack->add(*nip, "6", "Nip");

    builder = Gtk::Builder::create_from_file("../test/ui/nip.ui");
    auto nnip = Gtk::Builder::get_widget_derived<NuevoNip>(builder, "box_nip");
    Global::Widget::v_main_stack->add(*nnip, "7", "Nuevo Nip");

    Global::Widget::v_main_stack->set_transition_type(Gtk::StackTransitionType::ROTATE_LEFT_RIGHT);

    this->set_child(*Global::Widget::v_main_stack);
}

VMainWindow::~VMainWindow()
{
}
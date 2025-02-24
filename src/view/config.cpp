#include "view/config.hpp"

VConfig::VConfig(/* args */)
{
    auto builder = Gtk::Builder::create_from_file("../test/ui/conf/home.ui");

    this->set_orientation(Gtk::Orientation::HORIZONTAL);
    this->v_btn_back_config = builder->get_widget<Gtk::Button>("btn_back_config");
    this->v_left_box = builder->get_widget<Gtk::Box>("left_box");
    this->v_stack_side = builder->get_widget<Gtk::StackSidebar>("stackside");

    this->append(*v_left_box);

    v_stack_side->set_stack(v_stack);
    v_stack.set_margin(10);
    v_stack.set_hexpand();
    v_stack.set_transition_type(Gtk::StackTransitionType::SLIDE_UP_DOWN);

    //llenado de Vistas de Configuracion
    builder = Gtk::Builder::create_from_file("../test/ui/conf/general.ui");
    auto general = Gtk::Builder::get_widget_derived<General>(builder, "page_0");
    v_stack.add(*general,"0","General");

    builder = Gtk::Builder::create_from_file("../test/ui/conf/info.ui");
    auto info = Gtk::Builder::get_widget_derived<Info>(builder, "page_1");
    v_stack.add(*info,"1","Informacion");

    builder = Gtk::Builder::create_from_file("../test/ui/conf/empresa.ui");
    auto empresa = Gtk::Builder::get_widget_derived<Empresa>(builder, "page_2");
    v_stack.add(*empresa,"2","Empresa");

    builder = Gtk::Builder::create_from_file("../test/ui/conf/impresora.ui");
    auto impresora = Gtk::Builder::get_widget_derived<Impresora>(builder, "page_3");
    v_stack.add(*impresora,"3","Impresora");

    builder = Gtk::Builder::create_from_file("../test/ui/conf/roles.ui");
    auto roles = Gtk::Builder::get_widget_derived<Roles>(builder, "page_4");
    v_stack.add(*roles,"4","Roles");

    builder = Gtk::Builder::create_from_file("../test/ui/conf/wifi.ui");
    auto wifi = Gtk::Builder::get_widget_derived<Wifi>(builder, "page_5");
    v_stack.add(*wifi,"5","Wifi");

    auto detalles_validadores = Gtk::manage(new DetallesValidador);
    v_stack.add(*detalles_validadores,"6","Detalle Validadores");
    v_stack.set_interpolate_size(true);

    v_scroll.set_child(v_stack);
    v_scroll.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);

    this->append(v_scroll);
}

VConfig::~VConfig()
{
}
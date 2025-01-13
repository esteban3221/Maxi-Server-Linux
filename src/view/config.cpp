#include "view/config.hpp"

VConfig::VConfig(/* args */)
{
    auto builder = Gtk::Builder::create_from_file("../test/ui/conf/home.ui");

    this->set_orientation(Gtk::Orientation::HORIZONTAL);
    this->v_btn_back_config = builder->get_widget<Gtk::Button>("btn_back_config");
    this->v_left_box = builder->get_widget<Gtk::Box>("left_box");
    this->v_stack_side = builder->get_widget<Gtk::StackSidebar>("stackside");

    v_stack_side->set_stack(v_stack);
    v_stack.set_margin(10);
    v_stack.set_hexpand();
    v_stack.set_transition_type(Gtk::StackTransitionType::SLIDE_UP_DOWN);

    //llenado de Vistas de Configuracion
    auto detalles_validadores = Gtk::manage(new DetallesValidador);
    v_stack.add(*detalles_validadores,"0","Detalle Validadores");

    v_scroll.set_child(v_stack);
    v_scroll.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);

    this->append(*v_left_box);
    this->append(v_scroll);
}

VConfig::~VConfig()
{
}
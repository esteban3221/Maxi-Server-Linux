#include "view/config/validador.hpp"

VDetallesValidador::VDetallesValidador(/* args */)
{
    auto builder = Gtk::Builder::create_from_file("../test/ui/conf/validadores.ui");
    auto v_box_bill = Gtk::Builder::get_widget_derived<VValidador>(builder,"box_detalles_validador");
    v_box_bill->set_id_conf(1);

    builder = Gtk::Builder::create_from_file("../test/ui/conf/validadores.ui");
    auto v_box_coin = Gtk::Builder::get_widget_derived<VValidador>(builder,"box_detalles_validador");
    v_box_coin->set_id_conf(3);

    this->set_orientation(Gtk::Orientation::HORIZONTAL);
    this->append(*v_box_bill);
    this->append(*v_box_coin);
}

VDetallesValidador::~VDetallesValidador()
{
}
#include "view/config/validador.hpp"

VDetallesValidador::VDetallesValidador(/* args */) : v_box_header(Gtk::Orientation::HORIZONTAL),
                                                     v_box_contenedor(Gtk::Orientation::HORIZONTAL)
{
    this->set_spacing(5);

    auto builder = Gtk::Builder::create_from_file("../test/ui/conf/validadores.ui");

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

    this->append(v_box_header);
    this->append(v_lbl_subtitulo);

    v_box_bill = Gtk::Builder::get_widget_derived<VValidador>(builder, "box_detalles_validador");
    v_box_bill->set_id_conf(1);

    builder = Gtk::Builder::create_from_file("../test/ui/conf/validadores.ui");
    v_box_coin = Gtk::Builder::get_widget_derived<VValidador>(builder, "box_detalles_validador");
    v_box_coin->set_id_conf(3);

    v_box_contenedor.append(*v_box_bill);
    v_box_contenedor.append(*v_box_coin);
    v_box_contenedor.set_homogeneous();

    this->append(v_box_contenedor);
    this->set_orientation(Gtk::Orientation::VERTICAL);
}

VDetallesValidador::~VDetallesValidador()
{
}
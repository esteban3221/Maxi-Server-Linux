#pragma once
#include "view/config/base.validador.hpp"

class VDetallesValidador : public Gtk::Box
{
protected:
    VValidador *v_box_bill = nullptr, *v_box_coin = nullptr;
    Gtk::Box v_box_contenedor,v_box_header;
    Gtk::Label v_lbl_header, v_lbl_subtitulo;
    Gtk::Button v_btn_test_coneccion;
public:
    VDetallesValidador(/* args */);
    ~VDetallesValidador();
};

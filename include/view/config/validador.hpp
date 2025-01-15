#pragma once
#include "view/config/base.validador.hpp"

class VDetallesValidador : public Gtk::Box
{
protected:
    VValidador *v_box_bill = nullptr, *v_box_coin = nullptr;
public:
    VDetallesValidador(/* args */);
    ~VDetallesValidador();
};

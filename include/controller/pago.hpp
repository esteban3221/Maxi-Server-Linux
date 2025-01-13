#pragma once
#include "view/base.venta_pago.hpp"
#include <iostream>

class Pago final  : public BVentaPago
{
private:
    void on_btn_retry_click() override;
    void on_btn_cancel_click() override;
public:
    Pago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Pago();
};


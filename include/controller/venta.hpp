#pragma once
#include "view/base/venta_pago.hpp"
#include <iostream>

class Venta : public BVentaPago
{
private:
    void on_btn_retry_click() override;
    void on_btn_cancel_click() override;
public:
    Venta(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Venta();
};


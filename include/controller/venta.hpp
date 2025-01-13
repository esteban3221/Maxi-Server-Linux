#pragma once
#include "view/base.venta_pago.hpp"
#include <iostream>
#include "global.hpp"

class Venta final: public BVentaPago
{
private:
    void on_btn_retry_click() override;
    void on_btn_cancel_click() override;

    crow::response inicia(const crow::request &req);
    crow::response deten(const crow::request &req);
public:
    Venta(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Venta();
};
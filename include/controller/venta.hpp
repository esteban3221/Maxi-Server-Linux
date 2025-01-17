#pragma once
#include <iostream>

#include "view/base.venta_pago.hpp"
#include "c_sharp_validator.hpp"
#include "global.hpp"

namespace RestApp = Global::Rest;
class Venta final: public BVentaPago
{
private:
    Global::Async async_gui;

    void on_btn_retry_click() override;
    void on_btn_cancel_click() override;

    void func_poll(const std::string &status, const crow::json::rvalue &data);

    crow::response inicia(const crow::request &req);
    crow::response deten(const crow::request &req);
public:
    Venta(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Venta();
};
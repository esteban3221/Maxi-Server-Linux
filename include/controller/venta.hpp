#pragma once
#include <iostream>

#include "view/base.venta_pago.hpp"
#include "model/log.hpp"
#include "c_sharp_validator.hpp"
#include "controller/pago.hpp"
#include "controller/config/impresora.hpp"
#include "global.hpp"

namespace RestApp = Global::Rest;
class Venta final: public BVentaPago
{
private:
    Global::Async async_gui;

    void on_btn_retry_click() override;
    void on_btn_cancel_click() override;

    bool pago_poll(int ant_coin, int ant_bill);
    bool cancelado;

    int faltante;
    std::string estatus;
    void func_poll(const std::string &status, const crow::json::rvalue &data);
    Glib::RefPtr<Gio::ListStore<MLevelCash>> s_level_ant;

    crow::response inicia(const crow::request &req);
public:
    Venta(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Venta();
};
#pragma once
#include "view/base.venta_pago.hpp"
#include <iostream>
#include "global.hpp"
#include "log.hpp"
#include "controller/config/impresora.hpp"
#include "c_sharp_validator.hpp"

namespace RestApp = Global::Rest;
class Pago final  : public BVentaPago
{
private:
    Global::Async async_gui;
    void on_btn_retry_click() override;
    void on_btn_cancel_click() override;

    bool pago_poll(int ant_coin,int ant_bill);
    std::string estatus;

    crow::response inicia(const crow::request &req);
public:
    Pago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Pago();

    static void da_pago(int cambio, const sigc::slot<bool ()> &slot, const std::string &tipo, std::string &estatus);
    static inline std::atomic_int32_t faltante;
};


#pragma once
#include "view/base.venta_pago.hpp"
#include <iostream>
#include "global.hpp"
#include "c_sharp_validator.hpp"

namespace RestApp = Global::Rest;
class Pago final  : public BVentaPago
{
private:
    Global::Async async_gui;
    void on_btn_retry_click() override;
    void on_btn_cancel_click() override;

    sigc::connection conn;

    std::map<int, int> s_level_mon, s_level_bill;

    std::map<int, int> cantidad_recyclador(const Validator &val);
    void calcula_cambios(std::atomic_int32_t &salida_coin, std::atomic_int32_t &salida_bill, std::atomic_bool &terminado_coin, std::atomic_bool &terminado_bill);
    bool pago_poll();

    crow::response inicia(const crow::request &req);
public:
    Pago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Pago();
};


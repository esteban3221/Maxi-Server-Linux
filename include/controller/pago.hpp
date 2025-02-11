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
    std::atomic_int32_t faltante;
    std::chrono::steady_clock::time_point start_time;

    std::map<int, int> s_level_mon, s_level_bill;

    bool pago_poll(int ant_coin,int ant_bill);

    crow::response inicia(const crow::request &req);
public:
    Pago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Pago();
};


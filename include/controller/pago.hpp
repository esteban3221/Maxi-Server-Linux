#pragma once
#include "view/base.venta_pago.hpp"
#include <iostream>
#include "global.hpp"
#include "log.hpp"
#include "controller/config/impresora.hpp"
#include "c_sharp_validator.hpp"

namespace RestApp = Global::Rest;
class Pago final : public BVentaPago
{
private:
    Global::Async async_gui;
    void on_btn_retry_click() override;
    void on_btn_cancel_click() override;

    bool pago_poll(int ant_coin, int ant_bill);
    std::string estatus;

    crow::response inicia(const crow::request &req);
    crow::response inicia_manual(const crow::request &req);

    const std::unordered_map<int, int> map_bill = {
        {0, 20},
        {1, 50},
        {2, 100},
        {3, 200},
        {4, 500},
        {5, 1'000}};
    const std::unordered_map<int, int> map_coin = {
        {0, 1},
        {1, 2},
        {2, 5},
        {3, 10}};

public:
    Pago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Pago();

    static void da_pago(int cambio, const sigc::slot<bool()> &slot, const std::string &tipo, std::string &estatus);
    static void da_pago(const std::string &bill, const std::string &coin, const sigc::slot<bool()> &slot, const std::string &tipo, std::string &estatus);
    static inline std::atomic_int32_t faltante;
};

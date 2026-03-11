#pragma once
#include "view/base.venta_pago.hpp"
#include <iostream>
#include "global.hpp"
#include "log.hpp"
#include "controller/config/impresora.hpp"
#include "model/detalle_movimiento.hpp"
#include "core/hub_cash.hpp"

namespace RestApp = Global::Rest;
class Pago final : public BVentaPago
{
private:
    Global::Async async_gui;
    void on_btn_retry_click() override;
    void on_btn_cancel_click() override;
    void on_error(const std::string &device, const std::string &error);
    void on_credit(const std::string &, const std::string &, const crow::json::rvalue &data, size_t credito);

    crow::response inicia(const crow::request &req);
    crow::response inicia_manual(const crow::request &req);

    crow::response inicia_cambio(const crow::request &req);
    crow::response inicia_cambio_manual(const crow::request &req);
    crow::response termina_cambio_manual(const crow::request &req);
    crow::response cancelar_cambio_manual(const crow::request &req);

    CashHub &hub = CashHub::instance();
    Log log;
    Glib::RefPtr<MLog> t_log;
    void reset_log(const crow::json::rvalue &param, const std::string &type);
    std::condition_variable cv_finalizado;
    std::mutex mtx_espera;
    bool transaccion_terminada = false;

public:
    Pago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Pago();
};

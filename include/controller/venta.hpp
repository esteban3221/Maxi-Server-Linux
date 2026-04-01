#pragma once
#include <iostream>
#include <condition_variable>
#include <mutex>

#include "view/base.venta_pago.hpp"
#include "model/log.hpp"

#include "controller/pago.hpp"
#include "controller/config/impresora.hpp"
#include "model/detalle_movimiento.hpp"

#include "global.hpp"
#include "core/hub_cash.hpp"

class Venta final: public BVentaPago
{
private:
    Global::Async async_gui;
    CashHub &hub = CashHub::instance();
    Log log;

    void on_btn_retry_click() override;
    void on_btn_cancel_click() override;

    bool cancelado;
    std::condition_variable cv_finalizado;
    std::mutex mtx_espera;
    bool transaccion_terminada = false;

    crow::response inicia(const crow::request &req);
    crow::response deten(const crow::request &req);

    void on_event_credit(const std::string &device_id, const std::string &type, const crow::json::rvalue &data, size_t credito);
    void on_error(const std::string &device, const std::string &error);
    Glib::RefPtr<MLog> t_log;

    void reset_log(const crow::json::rvalue &param);

    bool is_view_ingreso;
    std::string concepto;

    // websocket
    crow::websocket::connection *connection{nullptr};
    void on_wb_socket_open(crow::websocket::connection &conn);
    void on_wb_socket_close(crow::websocket::connection &conn, const std::string &reason, uint16_t code);
    void on_wb_socket_message(crow::websocket::connection &conn, const std::string &data, bool is_binary);
public:
    Venta(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder, crow::SimpleApp& app);
    ~Venta();
};
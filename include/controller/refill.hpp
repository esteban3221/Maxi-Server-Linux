#pragma once
#include "view/refill.hpp"
#include "level_cash.hpp"

#include "log.hpp"
#include "controller/config/impresora.hpp"
#include <map>

class Refill final : public VRefill
{
protected:
    void on_show_map();
    void on_credit(const std::string &, const std::string &, const crow::json::rvalue &data, size_t credito);
    void on_error(const std::string &device, const std::string &error); // este es el canal para escuchar cuando se remueva el cashbox

private:
    Global::Async async_gui;
    CashHub &hub = CashHub::instance();
    Log log;

    crow::response inicia(const crow::request &req);
    crow::response deten_remoto(const crow::request &req);
    crow::response get_dashboard(const crow::request &req);
    crow::response update_imovilidad(const crow::request &req);

    crow::response transpaso(const crow::request &req);
    crow::response retirada(const crow::request &req);

    void deten();
    Glib::RefPtr<MLog> t_log;
    
    bool cancelado;
    size_t cashbox_level;
    std::condition_variable cv_finalizado;
    std::mutex mtx_espera;
    bool transaccion_terminada = false;
    void reset_log(const crow::json::rvalue &param);

    void init_data(Gtk::ColumnView *vcolumn, const std::string &tabla);
    size_t calcula_total(const std::string &type, const std::shared_ptr<Gio::ListStore<MLevelCash>> &list_store);
    // websocket
    void on_wb_socket_open(crow::websocket::connection &conn);
    void on_wb_socket_close(crow::websocket::connection &conn, const std::string &reason, uint16_t code);
    void on_wb_socket_message(crow::websocket::connection &conn, const std::string &data, bool is_binary);
    crow::websocket::connection *conn_ = nullptr;
    void envia_mensaje_wb(const std::string &device, const Glib::RefPtr<MLevelCash> &item);

public:
    Refill(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder, crow::SimpleApp& app);
    ~Refill();
};

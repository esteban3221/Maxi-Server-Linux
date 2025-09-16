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

    static void poll_pago(const std::pair<int, std::string> &);
    void poll_cambio(const std::string &status, const crow::json::rvalue &data);
    std::string estatus;
    std::string concepto;

    crow::response inicia(const crow::request &req);
    crow::response inicia_manual(const crow::request &req);

    crow::response inicia_cambio(const crow::request &req);
    crow::response inicia_cambio_manual(const crow::request &req);
    crow::response termina_cambio_manual(const crow::request &req);
    crow::response cancelar_cambio_manual(const crow::request &req);

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

    static void da_pago(int cambio, const std::string &tipo, std::string &estatus);
    static void da_pago(int cambio, const std::string &tipo, std::string &estatus, bool is_cambio);
    static void da_pago(const std::string &bill, const std::string &coin, const std::string &tipo, std::string &estatus);
    static inline std::atomic_int32_t faltante;

    std::tuple<int, std::vector<int>, std::vector<int>> procesar_parametros_iniciales(const crow::json::rvalue &bodyParams);
    void validar_inventario_disponible(const std::vector<int> &bill_values, const std::vector<int> &coin_values);
    void configurar_estado_pago(int cambio);
    int calcular_total_pago(const std::vector<int> &bill_values, const std::vector<int> &coin_values);
    void mostrar_interfaz_pago_manual(int total);
    Glib::RefPtr<MLog> registrar_pago_y_log(const std::vector<int> &bill_values, const std::vector<int> &coin_values, const std::string &tipo_pago);
    void imprimir_ticket_si_corresponde(const Glib::RefPtr<MLog> &t_log);
    crow::response finalizar_proceso_pago(const Glib::RefPtr<MLog> &t_log);
    std::string vector_to_json_array(const std::vector<int> &values);
};

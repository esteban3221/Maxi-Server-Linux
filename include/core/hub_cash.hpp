#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <glibmm.h>
#include <cpr/cpr.h>

#include "context/validador.hpp"
#include "model/level_cash.hpp"
#include "model/detalle_movimiento.hpp"
#include "controller/session.hpp"


namespace fs = std::filesystem;
enum class HttpMethod { GET, POST, PUT, DELETE };
enum class ValidadorType {BILL = 0, COIN = 16 }; // referencia del ssp

class CashHub
{
private:
    std::vector<std::unique_ptr<ValidadorUnit>> unidades;

    friend class Sesion;
    std::string autentica();

    // Señales Globales
    sigc::signal<void(const std::string &device_id, const std::string &type_val, const crow::json::rvalue &, size_t)> signal_credito;
    sigc::signal<void(std::string)> signal_hub_error;

    CashHub() = default;

    bool intentar_registrar(const std::string &puerto, int ssp);
    int obtener_ssp_por_serial(const std::string& puerto);
    crow::json::rvalue rutas_default(ValidadorUnit* val);

public:
    static CashHub &instance()
    {
        static CashHub instance;
        return instance;
    }

    // --- Escaneo Automático ---
    void inicializar_hardware();

    // Accessors para las señales centralizadas
    auto &on_credito() { return signal_credito; }
    auto &on_error() { return signal_hub_error; }

    //Pass-through
    std::map<std::string , cpr::Response> command_for_all(HttpMethod method, const std::string &command, const std::string &json = "", bool debug = false);
    cpr::Response command_by_device_id(HttpMethod method,const std::string &device_id, const std::string &command, const std::string &json = "", bool debug = false);
    void inicia_for_all(const Conf &conf,std::map<std::string, const crow::json::rvalue> = {});
    void inicia_poll_for_all();
    void inicia_pago(size_t monto, bool is_cambio = false);
    void inicia_pago(std::map<std::string , std::string>);
    void detiene_poll_for_all();
    void detiene_for_all(void);
};
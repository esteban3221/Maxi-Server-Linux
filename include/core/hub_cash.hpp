#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <glibmm.h>
#include "context/validador.hpp"
#include "cpr/cpr.h"

namespace fs = std::filesystem;
enum class HttpMethod { GET, POST, PUT, DELETE };
enum class ValidadorType {BILL = 0, COIN = 16 }; // referencia del ssp

class CashHub
{
private:
    std::vector<std::unique_ptr<ValidadorUnit>> unidades;
    std::string autentica();

    // Señales Globales: Cualquier parte de tu app se suscribe aquí
    sigc::signal<void(const crow::json::rvalue &, size_t)> signal_credito;
    sigc::signal<void(std::string)> signal_hub_error;

    CashHub() = default;

    bool intentar_registrar(const std::string &puerto, int ssp);
    int obtener_ssp_por_serial(const std::string& puerto);

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
    void inicia_for_all(const Conf &conf,std::map<std::string, const crow::json::rvalue>);
    void inicia_poll_for_all();
    void inicia_pago(size_t monto, bool is_cambio = false);
    void inicia_pago(std::map<std::string , std::string>);
    void detiene_poll_for_all();
    void detiene_for_all(void);
};
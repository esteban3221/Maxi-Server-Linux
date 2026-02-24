#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include "context/validador.hpp"
#include "cpr/cpr.h"

namespace fs = std::filesystem;

class CashHub
{
private:
    std::vector<std::unique_ptr<ValidadorUnit>> unidades;
    std::string autentica();

    // Señales Globales: Cualquier parte de tu app se suscribe aquí
    sigc::signal<void(ValidadorUnit *, int)> signal_credito_billete;
    sigc::signal<void(ValidadorUnit *, int)> signal_credito_moneda;
    sigc::signal<void(std::string)> signal_hub_error;

    CashHub() = default;

public:
    static CashHub &instance()
    {
        static CashHub instance;
        return instance;
    }

    // --- Escaneo Automático ---
    void inicializar_hardware();

private:
    bool intentar_registrar(const std::string &puerto, int ssp);
public:
    // Accessors para las señales centralizadas
    auto &on_billete() { return signal_credito_billete; }
    auto &on_moneda() { return signal_credito_moneda; }
};
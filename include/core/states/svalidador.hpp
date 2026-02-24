//"states/svalidador.hpp"

/*
Idle → Iniciando → Conectando → Activo → Pausado → Deteniendo → Idle
  ↑                                                    |
  └────────────────────────────────────────────────────┘

*/

#pragma once
#include <utility>
#include <crow.h>
#include "interfaces/ivalidador.hpp"

class ValidadorUnit;
struct Conf;

// --- Estado: Idle ---
class EstadoIdle final : public IValidador {
public:
    void on_entry(ValidadorUnit &v) override;
    void on_start_init(ValidadorUnit &v, const Conf &conf, const crow::json::rvalue &set_routes) override;
    void on_handle_event(ValidadorUnit &v, const std::string &e, const crow::json::rvalue &d) override;
    std::string get_nombre_estado() const override { return "Idle"; }
};

// --- Estado: Iniciando ---
class EstadoIniciando final : public IValidador {
public:
    void on_entry(ValidadorUnit &v) override;
    void on_start_init(ValidadorUnit &v, const Conf &conf, const crow::json::rvalue &set_routes) override;
    void on_handle_event(ValidadorUnit &v, const std::string &e, const crow::json::rvalue &d) override;
    std::string get_nombre_estado() const override { return "Iniciando"; }
};

// --- Estado: Activo ---
class EstadoActivo final : public IValidador {
public:
    void on_entry(ValidadorUnit &v) override;
    void on_stop_disconnect(ValidadorUnit &v) override;
    void on_handle_event(ValidadorUnit &v, const std::string &event_type, const crow::json::rvalue &data) override;
    std::string get_nombre_estado() const override { return "Activo"; }
};

// --- Estado: Deteniendo ---
class EstadoDeteniendo final : public IValidador {
public:
    void on_entry(ValidadorUnit &v) override;
    void on_handle_event(ValidadorUnit &v, const std::string &e, const crow::json::rvalue &d) override;
    std::string get_nombre_estado() const override { return "Deteniendo"; }
};

// --- Estado: Error ---
class EstadoError final : public IValidador {
private:
    std::string mensaje;
public:
    EstadoError(std::string m) : mensaje(std::move(m)) {}
    void on_entry(ValidadorUnit &v) override;
    void on_handle_event(ValidadorUnit &v, const std::string &e, const crow::json::rvalue &d) override;
    std::string get_nombre_estado() const override { return "Error"; }
};
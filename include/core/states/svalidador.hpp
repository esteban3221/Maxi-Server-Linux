//"states/svalidador.hpp"
#pragma once

#include <utility>
#include <crow.h>

#include "interfaces/ivalidador.hpp"

/*
Idle → Iniciando → Conectando → Activo → Pausado → Deteniendo → Idle
  ↑                                                    |
  └────────────────────────────────────────────────────┘
*/
class ValidadorUnit;



class EstadoIniciando final : public IValidador
{
public:
    std::string get_nombre_estado() const override { return "Iniciando"; }
    void on_handle_event(ValidadorUnit& v, const std::string& event_type,  const crow::json::rvalue& data) override {

    }
};

class EstadoDeteniendo final : public IValidador
{
public:
    std::string get_nombre_estado() const override { return "Deteniendo"; }
    void on_handle_event(ValidadorUnit& v, const std::string& event_type,  const crow::json::rvalue& data) override {

    }
};

class EstadoIdle final : public IValidador {
public:
    void on_entry(ValidadorUnit& v) override {
        CROW_LOG_INFO << "Entrando a estado Idle" << '\n';
    }
    
    void on_start_init(ValidadorUnit& v) override {
        CROW_LOG_INFO << "Iniciando desde Idle" << '\n';
        v.transiciona_estado(std::make_unique<EstadoIniciando>());
    }
    
    void on_handle_event(ValidadorUnit& v, const std::string& event_type,  const crow::json::rvalue& data) override {
        CROW_LOG_WARNING << "No se esperan eventos en estado Idle. Evento recibido: " << event_type << '\n';
    }
    
    std::string get_nombre_estado() const override { return "Idle"; }
    bool puede_iniciar() const override { return true; }
    bool puede_detener() const override { return false; }
};

// states/estado_activo.hpp
class EstadoActivo final : public IValidador {
private:
    std::thread polling_thread;
    std::atomic<bool> running{true};
    
public:
    void on_entry(ValidadorUnit& v) override {
        CROW_LOG_INFO << "Entrando a estado Activo" << '\n';
        // Iniciar polling cuando entramos al estado
        iniciar_polling(v);
    }
    
    void on_exit(ValidadorUnit& v) override {
        CROW_LOG_INFO << "Saliendo de estado Activo" << '\n';
        detener_polling();
    }
    
    void on_stop_disconnect(ValidadorUnit& v) override {
        v.transiciona_estado(std::make_unique<EstadoDeteniendo>());
    }
    
    void on_handle_event(ValidadorUnit& v, const std::string& event_type, 
                        const crow::json::rvalue& data) override {
        // if (event_type == "credito_recibido") {
        //     v.ingreso_credito = data["monto"].i();
        //     v.signal_event_received.emit("credito", data);
        // } else if (event_type == "comando_detener") {
        //     on_stop_disconnect(v);
        // }
    }
    
    std::string get_nombre_estado() const override { return "Activo"; }
    bool puede_iniciar() const override { return false; }
    bool puede_detener() const override { return true; }
    
private:
    void iniciar_polling(ValidadorUnit& v) {
        polling_thread = std::thread([this, &v]() {
            while (running) {
                // auto resp = v.command_get("poll");
                // if (resp.status_code == 200) {
                //     auto json = crow::json::load(resp.text);
                //     if (json && json.has("evento")) {
                //         v.on_handle_event(json["evento"].s(), json);
                //     }
                // }
                // std::this_thread::sleep_for(std::chrono::milliseconds(v.poll_milli));
            }
        });
    }
    
    void detener_polling() {
        running = false;
        if (polling_thread.joinable()) {
            polling_thread.join();
        }
    }
};



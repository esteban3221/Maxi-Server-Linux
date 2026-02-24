//"context/validador.hpp"

#pragma once
// #include "states/svalidador.hpp"
#include "interfaces/ivalidador.hpp"

#include <sigc++/sigc++.h>
#include <utility>
#include <iostream>
#include <crow.h>
#include <cpr/cpr.h>

// Forward declaration de los estados
class EstadoIdle;
class EstadoIniciando;
class EstadoConectando;
class EstadoActivo;
class EstadoPausado;
class EstadoDeteniendo;

class ValidadorUnit
{
private:
    std::unique_ptr<IValidador> handler_state;

    std::string nombre_validator;

    size_t ingreso_credito;
    size_t salida_credito;
    size_t poll_milli;

    struct Conf
    {
        std::string puerto = "/dev/ttyUSB";
        uint32_t ssp = 0;
        std::string dispositivo;
        std::string log_ruta;
        bool habilita_recolector;
        bool auto_acepta_credito;
        bool habilita_salida_credito;
    };
    std::atomic<bool> poll;

    // utilidades para consumir la api rest del validador
    const std::string BASE_URL = "http://localhost:5000/api/CashDevice/";
    void imprime_debug(const cpr::Response &r) const;
    const cpr::Response command_get(const std::string &command, bool debug = false) const;
    const cpr::Response command_post(const std::string &command, const std::string &json = "", bool = false) const;

public:
    ValidadorUnit(/* args */);
    ~ValidadorUnit();

    sigc::signal<void(const std::string&)> signal_state_changed;       // Emite nuevo estado
    sigc::signal<void(const std::string&, const crow::json::rvalue&)> signal_event_received;  // Eventos del poll
    sigc::signal<void(const std::string&)> signal_error;               // Errores graves

    void inicia_conecta(const Conf &conf, const crow::json::rvalue &set_routes);
    void detiene_desconecta();

    void transiciona_estado(std::unique_ptr<IValidador> nuevo_estado);
    void on_handle_event(const std::string &event_type, const crow::json::rvalue &data);
};
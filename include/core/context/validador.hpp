//"context/validador.hpp"

#pragma once

#include <sigc++/sigc++.h>
#include <utility>
#include <memory>  // Necesario para unique_ptr
#include <atomic>
#include <string>
#include <crow.h>
#include <cpr/cpr.h>

#include "interfaces/ivalidador.hpp"

// Forward declaration de los estados
class EstadoIdle;
class EstadoIniciando;
class EstadoActivo;
class EstadoPausado;
class EstadoDeteniendo;

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

class ValidadorUnit
{
private:
    std::unique_ptr<IValidador> handler_state;

    std::string device_id;
    std::string device_model;

    size_t ingreso_credito;
    size_t salida_credito;
    size_t poll_milli;

    std::atomic<bool> poll;
    std::string token;
    Conf conf;
    bool esperar_pago_async();

    // utilidades para consumir la api rest del validador
    const std::string BASE_URL = "http://localhost:5000/api/v2/CashDevice/";
    void imprime_debug(const std::string &command, const cpr::Response &r, const std::string &body = "") const;
    std::string ultimo_cash_level;

public:
    ValidadorUnit(/* args */);
    ~ValidadorUnit();

    sigc::signal<void(const std::string &)> signal_state_changed;                              // Emite nuevo estado
    sigc::signal<void(const std::string &, const std::string &, const std::string &, const crow::json::rvalue &)> signal_event_received; // Eventos del poll
    sigc::signal<void(const std::string &, const std::string &)> signal_error;                                      // Errores graves

    //propertys encapsulamiento
    
    size_t &property_poll_milli() { return poll_milli; }
    std::atomic<bool> &property_poll() { return poll; }
    size_t &property_ingreso_credito() { return ingreso_credito; }
    size_t &property_salida_credito() { return salida_credito; }
    Conf &property_conf(){return conf;}
    std::string &property_token() { return token; }
    std::string &property_ultimo_cash_level(){ return ultimo_cash_level; }
    const std::string &property_device_model() {return device_model;} //solo_lectura
    const std::string &property_device_id() {return device_id;} //solo_lectura

    //devuelve los niveles actuales de ingreso y salida al conectar
    const crow::json::rvalue inicia_conecta (const crow::json::rvalue &set_routes);
    void detiene_desconecta();
    void iniciar_polling();
    void iniciar_pago(const std::string &);
    uint iniciar_pago(size_t, bool is_cambio, const std::string &actual_level);
    crow::json::wvalue obten_cambio(uint &cambio, std::map<int, int> &reciclador, bool is_cambio);
    const std::string get_nombre_estado();

    void transiciona_estado(std::unique_ptr<IValidador> nuevo_estado);

    const cpr::Response command_get(const std::string &command, bool debug = false) const;
    const cpr::Response command_post(const std::string &command, const std::string &json = "", bool = false) const;
};
#pragma once

#include <sigc++/sigc++.h>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <string>
#include <mutex>
#include <vector>
#include "essp_driver/src/lib.rs.h"

struct Conf
{
    std::string puerto = "/dev/ttyUSB0";
    uint32_t ssp = 0;
    bool habilita_recolector = false;
    bool auto_acepta_credito = false;
    bool habilita_salida_credito = false;
};

class ValidadorUnit
{
private:
    std::string device_id;
    std::string device_model;

    size_t poll_milli = 100;

    std::atomic<bool> poll{false};
    std::jthread poll_thread;
    std::condition_variable_any cv_poll;

    mutable std::mutex mtx_comunicacion;
    mutable rust::Box<ffi::RustEsspUnit> rust_unit;

    Conf conf;
    std::vector<ffi::DenomLevel> ultimo_cash_level;

    bool esperar_pago_fisico();

public:
    ValidadorUnit();
    ~ValidadorUnit();

    sigc::signal<void(const ffi::CashEvent &)> signal_event_received;
    sigc::signal<void(const std::string &, const std::string &)> signal_error;

    size_t &property_poll_milli() { return poll_milli; }
    std::atomic<bool> &property_poll() { return poll; }
    Conf &property_conf() { return conf; }
    const std::string &property_device_id() const { return device_id; }
    const std::string &property_device_model() const { return device_model; }
    ffi::DeviceType property_device_type() const { return conf.ssp == 16 ? ffi::DeviceType::Coin : ffi::DeviceType::Bill; }
    const std::vector<ffi::DenomLevel> &property_ultimo_cash_level() const { return ultimo_cash_level; }

    bool inicia_conecta(const std::vector<ffi::RouteRule> &rutas = {});
    void detiene_desconecta();
    void iniciar_polling();
    void detener_polling();
    bool reiniciar();

    std::vector<ffi::DenomLevel> get_nivel_actual();
    bool cambiar_ruta(int32_t valor_pesos, bool a_reciclador);
    bool vaciar_reciclador(bool smart_empty = true);
    void detener_pago();
    

    uint32_t iniciar_pago_automatico(uint32_t monto_pesos, bool is_cambio);
    bool iniciar_pago_manual(const std::vector<ffi::PayoutItem> &piezas);
    std::vector<ffi::PayoutItem> calcular_cambio(uint32_t &cambio_restante, const std::vector<ffi::DenomLevel> &niveles, bool is_cambio) const;

    bool aceptar_escrow();
    bool rechazar_escrow();
    void limpiar_niveles_cashbox();
    bool iniciar_pago_por_canales(const std::vector<int32_t> &cantidades);
};
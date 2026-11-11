#pragma once

#include "core/context/validador.hpp"
#include "model/level_cash.hpp"
#include "model/detalle_movimiento.hpp"
#include "shared/global.hpp"
#include <giomm/liststore.h>
#include <sigc++/sigc++.h>
#include <filesystem>
#include <utility>
#include <memory>
#include <vector>
#include <string>
#include <atomic>
#include <map>

namespace fs = std::filesystem;

struct DeviceSnapshot
{
    ffi::DeviceType type;
    std::vector<ffi::DenomLevel> levels;
};

class CashHub
{
private:
    std::vector<std::unique_ptr<ValidadorUnit>> unidades;
    decltype(std::declval<LevelCash>().get_level_cash()) m_list_billetes;
    DetalleMovimiento detalle;

    sigc::signal<void(const ffi::CashEvent &)> signal_credito;
    sigc::signal<void(const std::string &, const std::string &)> signal_hub_error;

    CashHub() = default;
    ~CashHub() { detiene_for_all(); }

    bool intentar_registrar(const std::string &puerto, int ssp);
    std::vector<ffi::RouteRule> rutas_default(const ValidadorUnit *val);

public:
    CashHub(const CashHub &) = delete;
    CashHub &operator=(const CashHub &) = delete;

    static CashHub &instance()
    {
        static CashHub inst;
        return inst;
    }

    std::atomic<bool> is_in_process{false};

    auto &on_credito() { return signal_credito; }
    auto &on_error() { return signal_hub_error; }

    std::string autentica() { return "native-rust-driver"; }

    void inicializar_hardware();

    void inicia_for_all(const Conf &conf = {}, const std::map<std::string, std::vector<ffi::RouteRule>> &set_routes = {});
    void detiene_for_all();

    void inicia_poll_for_all();
    void detiene_poll_for_all(size_t t_id);
    void reiniciar_for_all();

    void inicia_pago(size_t t_id, size_t monto_pesos, bool is_cambio = false);
    void inicia_pago(size_t t_id, const std::map<std::string, std::vector<ffi::PayoutItem>> &pagos_manuales);
    void inicia_pago_por_canales(size_t t_id, const std::map<std::string, std::vector<int32_t>> &mapa_canales);

    std::map<std::string, DeviceSnapshot> obten_ultimo_snapshot_level() const;
    std::map<std::string, DeviceSnapshot> obten_niveles_actuales_for_all();
    std::vector<ffi::DenomLevel> get_nivel_actual_by_id(const std::string &device_id) const;

    bool aceptar_escrow(const std::string &device_id);
    bool rechazar_escrow(const std::string &device_id);
    bool vaciar_dispositivo(const std::string &device_id, bool smart_empty);
    void limpiar_cashbox(const std::string &device_id);
};
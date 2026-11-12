#include "core/context/validador.hpp"
#include <crow/logging.h>
#include <algorithm>
#include <chrono>

ValidadorUnit::ValidadorUnit()
    : device_id("Desconocido"),
      device_model("ITL-eSSP"),
      rust_unit(ffi::new_essp_unit("/dev/ttyUSB0", 0))
{
}

ValidadorUnit::~ValidadorUnit()
{
    detiene_desconecta();
}

bool ValidadorUnit::inicia_conecta(const std::vector<ffi::RouteRule> &rutas)
{
    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    try
    {
        rust_unit = ffi::new_essp_unit(conf.puerto, conf.ssp);
        rust::Slice<const ffi::RouteRule> slice_rutas(rutas.data(), rutas.size());

        auto niveles_rust = rust_unit->iniciar_y_conectar(conf.habilita_recolector, slice_rutas);

        device_id = std::string(rust_unit->get_serial_id());
        device_model = (conf.ssp == 16) ? "ITL-Coin-Hopper" : "ITL-Bill-Recycler";

        ultimo_cash_level.assign(niveles_rust.begin(), niveles_rust.end());
        CROW_LOG_INFO << "Conectado nativamente a " << device_model << " [ID: " << device_id << "]";
        return true;
    }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << "Error al conectar en " << conf.puerto << ": " << e.what();
        return false;
    }
}

bool ValidadorUnit::reiniciar()
{
    detener_polling();
    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    try
    {
        rust_unit->reiniciar();
        return true;
    }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << "Error reiniciando unidad " << device_id << ": " << e.what();
        return false;
    }
}


void ValidadorUnit::detener_polling()
{
    poll.store(false, std::memory_order_release);
    cv_poll.notify_all();

    if (poll_thread.joinable())
    {
        poll_thread.request_stop();
        if (poll_thread.get_id() != std::this_thread::get_id())
        {
            poll_thread.join();
        }
    }
}

void ValidadorUnit::detiene_desconecta()
{
    detener_polling();
    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    rust_unit->detener_y_desconectar();
}

std::vector<ffi::DenomLevel> ValidadorUnit::get_nivel_actual()
{
    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    try
    {
        auto niveles_rust = rust_unit->obtener_niveles();
        ultimo_cash_level.assign(niveles_rust.begin(), niveles_rust.end());
    }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << "No se pudieron leer niveles de " << device_id << ": " << e.what();
    }
    return ultimo_cash_level;
}

bool ValidadorUnit::cambiar_ruta(int32_t valor_pesos, bool a_reciclador)
{
    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    try
    {
        rust_unit->cambiar_ruta(valor_pesos * 100, a_reciclador);
        return true;
    }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << "Error cambiando ruta en " << device_id << ": " << e.what();
        return false;
    }
}

bool ValidadorUnit::vaciar_reciclador(bool smart_empty)
{
    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    try
    {
        rust_unit->vaciar_reciclador(smart_empty);
        return true;
    }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << "Error vaciando reciclador en " << device_id << ": " << e.what();
        return false;
    }
}

void ValidadorUnit::detener_pago()
{
    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    try
    {
        rust_unit->detener_pago();
    }
    catch (...) {}
}

void ValidadorUnit::iniciar_polling()
{
    if (poll.load(std::memory_order_acquire))
        return;

    poll.store(true, std::memory_order_release);
    poll_thread = std::jthread([this](std::stop_token stoken)
    {
        std::mutex mtx_sleep;

        while (!stoken.stop_requested() && poll.load(std::memory_order_acquire))
        {
            rust::Vec<ffi::CashEvent> eventos;
            bool error_comm = false;

            {
                std::lock_guard<std::mutex> lock(mtx_comunicacion);
                try
                {
                    eventos = rust_unit->hacer_poll();
                }
                catch (const std::exception &e)
                {
                    CROW_LOG_ERROR << "Error serial en poll (" << device_id << "): " << e.what();
                    error_comm = true;
                }
            }

            if (error_comm)
            {
                signal_error.emit(device_id, "Perdida de comunicacion serial con el validador");
            }
            else
            {
                for (const auto &ev : eventos)
                {
                    switch (ev.kind)
                    {
                    case ffi::EventKind::Credit:
                        CROW_LOG_INFO << "Credito recibido en " << device_id << ": $" << ev.value_pesos;
                        signal_event_received.emit(ev);
                        break;
                    case ffi::EventKind::FraudAttempt:
                        signal_error.emit(device_id, "Intento de fraude detectado");
                        break;
                    case ffi::EventKind::Jammed:
                        signal_error.emit(device_id, "Billete o moneda atascada");
                        break;
                    case ffi::EventKind::CashboxRemoved:
                        signal_error.emit(device_id, "CASHBOX_REMOVED");
                        break;
                    case ffi::EventKind::CashboxReplaced:
                        signal_error.emit(device_id, "CASHBOX_REPLACED");
                        break;
                    default:
                        break;
                    }
                }
            }

            std::unique_lock<std::mutex> ulock(mtx_sleep);
            cv_poll.wait_for(ulock, stoken, std::chrono::milliseconds(poll_milli), [this]() {
                return !poll.load(std::memory_order_acquire);
            });
        }
        CROW_LOG_INFO << "Polling finalizado limpiamente para " << device_id;
    });
}

bool ValidadorUnit::esperar_pago_fisico()
{
    auto inicio = std::chrono::steady_clock::now();
    const auto timeout_max = std::chrono::seconds(25);
    bool exito = false;
    bool terminado = false;

    while (!terminado)
    {
        if (std::chrono::steady_clock::now() - inicio > timeout_max)
        {
            CROW_LOG_WARNING << "Timeout de software alcanzado en entrega de " << device_id << ". Deteniendo motor (0x38)...";
            detener_pago();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            {
                std::lock_guard<std::mutex> lock(mtx_comunicacion);
                try { rust_unit->hacer_poll(); } catch (...) {}
            }
            return false;
        }

        {
            std::lock_guard<std::mutex> lock(mtx_comunicacion);
            try
            {
                auto eventos = rust_unit->hacer_poll();
                for (const auto &ev : eventos)
                {
                    if (ev.kind == ffi::EventKind::Dispensed)
                    {
                        exito = true;
                        terminado = true;
                    }
                    else if (ev.kind == ffi::EventKind::Timeout ||
                             ev.kind == ffi::EventKind::IncompletePayout ||
                             ev.kind == ffi::EventKind::Jammed ||
                             ev.kind == ffi::EventKind::Error)
                    {
                        exito = false;
                        terminado = true;
                    }
                }
            }
            catch (...) {}
        }

        if (!terminado)
            std::this_thread::sleep_for(std::chrono::milliseconds(poll_milli));
    }
    return exito;
}

std::vector<ffi::PayoutItem> ValidadorUnit::calcular_cambio(
    uint32_t &cambio_restante,
    const std::vector<ffi::DenomLevel> &niveles,
    bool is_cambio) const
{
    std::vector<ffi::PayoutItem> a_dispensar;
    const uint32_t monto_original = cambio_restante;

    auto niveles_ordenados = niveles;
    std::sort(niveles_ordenados.begin(), niveles_ordenados.end(),
              [](const ffi::DenomLevel &a, const ffi::DenomLevel &b) {
                  return a.value_pesos > b.value_pesos;
              });

    for (auto &denom : niveles_ordenados)
    {
        if (denom.value_pesos <= 0 || denom.stored_in_payout <= 0)
            continue;

        uint32_t valor = static_cast<uint32_t>(denom.value_pesos);
        int32_t disponibles = denom.stored_in_payout;
        int32_t piezas_usar = 0;

        while (cambio_restante >= valor &&
               (!is_cambio || valor < monto_original) &&
               disponibles > 0)
        {
            cambio_restante -= valor;
            disponibles--;
            piezas_usar++;
        }

        if (piezas_usar > 0)
        {
            a_dispensar.push_back(ffi::PayoutItem{
                .value_cents = denom.value_cents,
                .count = piezas_usar
            });
        }
    }
    return a_dispensar;
}

uint32_t ValidadorUnit::iniciar_pago_automatico(uint32_t monto_pesos, bool is_cambio)
{
    uint32_t remanente = monto_pesos;
    auto niveles = get_nivel_actual();
    auto piezas = calcular_cambio(remanente, niveles, is_cambio);

    if (!piezas.empty())
    {
        iniciar_pago_manual(piezas);
    }
    return remanente;
}

bool ValidadorUnit::iniciar_pago_manual(const std::vector<ffi::PayoutItem> &piezas)
{
    if (piezas.empty())
        return true;

    try
    {
        {
            std::lock_guard<std::mutex> lock(mtx_comunicacion);
            rust::Slice<const ffi::PayoutItem> slice(piezas.data(), piezas.size());
            rust_unit->dispensar_piezas(slice);
        }

        bool ok = esperar_pago_fisico();
        // Siempre actualizamos los niveles reales para registrar salidas parciales si hubo timeout/atasco
        get_nivel_actual();

        if (!ok)
        {
            signal_error.emit(device_id, "Atasco o tiempo agotado durante la entrega de efectivo.");
            return false;
        }
        return true;
    }
    catch (const std::exception &e)
    {
        CROW_LOG_CRITICAL << device_id << " -> Error al dispensar: " << e.what();
        get_nivel_actual();
        signal_error.emit(device_id, e.what());
        return false;
    }
}

bool ValidadorUnit::aceptar_escrow()
{
    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    try { rust_unit->aceptar_escrow(); return true; }
    catch (...) { return false; }
}

bool ValidadorUnit::rechazar_escrow()
{
    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    try { rust_unit->rechazar_escrow(); return true; }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << "Error rechazando billete en " << device_id << ": " << e.what();
        return false;
    }
}

void ValidadorUnit::limpiar_niveles_cashbox()
{
    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    rust_unit->limpiar_niveles_cashbox();
}

bool ValidadorUnit::iniciar_pago_por_canales(const std::vector<int32_t> &cantidades)
{
    bool tiene_piezas = std::any_of(cantidades.begin(), cantidades.end(), [](int32_t c) { return c > 0; });
    if (!tiene_piezas)
        return true;

    try
    {
        {
            std::lock_guard<std::mutex> lock(mtx_comunicacion);
            rust::Slice<const int32_t> slice(cantidades.data(), cantidades.size());
            rust_unit->dispensar_por_canales(slice);
        }

        bool ok = esperar_pago_fisico();
        get_nivel_actual();

        if (!ok)
        {
            signal_error.emit(device_id, "Atasco o tiempo agotado durante la entrega de efectivo.");
            return false;
        }
        return true;
    }
    catch (const std::exception &e)
    {
        CROW_LOG_CRITICAL << device_id << " -> Error al dispensar por canales: " << e.what();
        get_nivel_actual();
        signal_error.emit(device_id, e.what());
        return false;
    }
}
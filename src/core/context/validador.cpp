#include "core/context/validador.hpp"

#include <crow/logging.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <vector>

namespace
{
constexpr int MAX_ERRORES_SERIAL_POLL = 5;
constexpr int MAX_ERRORES_SERIAL_PAGO = 5;
constexpr auto TIMEOUT_INACTIVIDAD_PAGO = std::chrono::seconds(18);
constexpr auto TIMEOUT_GLOBAL_PAGO = std::chrono::seconds(60);
constexpr int MAX_POLLS_DRENADO_HALT = 15; // 15 * 150ms = 2.25 s máx. tras 0x38
constexpr int POLLS_QUIETOS_TRAS_HALT = 3; // 450 ms sin actividad física tras 0x38
constexpr uint32_t MAX_NODOS_DFS_CAMBIO = 200000;

uint32_t calcular_entregado_por_niveles(
    const std::vector<ffi::DenomLevel> &antes,
    const std::vector<ffi::DenomLevel> &despues)
{
    std::unordered_map<int32_t, int32_t> stock_despues;
    stock_despues.reserve(despues.size());
    for (const auto &lvl : despues)
    {
        stock_despues[lvl.value_cents] = lvl.stored_in_payout;
    }

    uint64_t total_pesos = 0;
    for (const auto &lvl : antes)
    {
        if (lvl.value_pesos <= 0)
            continue;
        auto it = stock_despues.find(lvl.value_cents);
        if (it != stock_despues.end() && lvl.stored_in_payout > it->second)
        {
            uint64_t salieron = static_cast<uint64_t>(lvl.stored_in_payout - it->second);
            total_pesos += salieron * static_cast<uint64_t>(lvl.value_pesos);
        }
    }

    return static_cast<uint32_t>(
        std::min<uint64_t>(total_pesos, std::numeric_limits<uint32_t>::max()));
}
} // namespace

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
    detener_polling();

    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    rust::Slice<const ffi::RouteRule> slice_rutas(rutas.data(), rutas.size());

    if (rust_unit->esta_conectado() &&
        puerto_conectado == conf.puerto &&
        ssp_conectado == conf.ssp)
    {
        try
        {
            auto niveles_rust =
                rust_unit->configurar_admision(conf.habilita_recolector, slice_rutas);
            ultimo_cash_level.assign(niveles_rust.begin(), niveles_rust.end());
            CROW_LOG_INFO << "Sesion AES persistente reutilizada en " << device_model
                          << " [ID: " << device_id << "]";
            return true;
        }
        catch (const std::exception &e)
        {
            CROW_LOG_WARNING << "Sesion persistente invalida en " << device_id
                             << " (" << e.what() << "). Reconectando desde cero...";
            rust_unit->detener_y_desconectar();
            puerto_conectado.clear();
            ssp_conectado = -1;
        }
    }

    // 2. Camino completo (arranque inicial o recuperación tras reset/desconexión USB)
    try
    {
        rust_unit = ffi::new_essp_unit(conf.puerto, conf.ssp);
        auto niveles_rust = rust_unit->iniciar_y_conectar(conf.habilita_recolector, slice_rutas);

        puerto_conectado = conf.puerto;
        ssp_conectado = conf.ssp;
        device_id = std::string(rust_unit->get_serial_id());
        device_model = (rust_unit->get_device_type() == ffi::DeviceType::Coin)
                           ? "ITL-Coin-Hopper"
                           : "ITL-Bill-Recycler";

        ultimo_cash_level.assign(niveles_rust.begin(), niveles_rust.end());
        CROW_LOG_INFO << "Conectado nativamente a " << device_model << " [ID: " << device_id << "]";
        return true;
    }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << "Error al conectar en " << conf.puerto << ": " << e.what();
        puerto_conectado.clear();
        ssp_conectado = -1;
        return false;
    }
}

void ValidadorUnit::pausar_sesion()
{
    detener_polling();
    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    if (!rust_unit->esta_conectado())
        return;

    try
    {
        rust_unit->pausar_sesion();
    }
    catch (const std::exception &e)
    {
        CROW_LOG_WARNING << "Error pausando sesion en " << device_id
                         << " (" << e.what() << "). Cerrando puerto para proxima reconexion.";
        rust_unit->detener_y_desconectar();
        puerto_conectado.clear();
        ssp_conectado = -1;
    }
}

bool ValidadorUnit::reiniciar()
{
    detener_polling();
    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    try
    {
        rust_unit->reiniciar();
        puerto_conectado.clear();
        ssp_conectado = -1;
        return true;
    }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << "Error reiniciando unidad " << device_id << ": " << e.what();
        puerto_conectado.clear();
        ssp_conectado = -1;
        return false;
    }
}

void ValidadorUnit::detiene_desconecta()
{
    detener_polling();
    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    rust_unit->detener_y_desconectar();
    puerto_conectado.clear();
    ssp_conectado = -1;
}

void ValidadorUnit::detener_polling()
{
    {
        std::lock_guard<std::mutex> lock(mtx_poll);
        poll.store(false, std::memory_order_release);
    }
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
    detener_polling();
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
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << "Error enviando HaltPayout (0x38) a " << device_id << ": " << e.what();
    }
}

void ValidadorUnit::iniciar_polling()
{
    {
        std::lock_guard<std::mutex> lock(mtx_poll);
        if (poll.load(std::memory_order_acquire))
            return;
        poll.store(true, std::memory_order_release);
    }

    // Si existía un hilo anterior que terminó por error serial, lo unimos antes de crear el nuevo.
    if (poll_thread.joinable() && poll_thread.get_id() != std::this_thread::get_id())
    {
        poll_thread.join();
    }

    poll_thread = std::jthread([this](std::stop_token stoken)
    {
        int errores_consecutivos = 0;

        while (!stoken.stop_requested() && poll.load(std::memory_order_acquire))
        {
            rust::Vec<ffi::CashEvent> eventos;
            std::string error_serial;

            {
                std::lock_guard<std::mutex> lock(mtx_comunicacion);
                try
                {
                    eventos = rust_unit->hacer_poll();
                }
                catch (const std::exception &e)
                {
                    error_serial = e.what();
                }
            }

            if (!error_serial.empty())
            {
                ++errores_consecutivos;
                CROW_LOG_ERROR << "Error serial en poll (" << device_id << ") ["
                               << errores_consecutivos << "/" << MAX_ERRORES_SERIAL_POLL
                               << "]: " << error_serial;

                if (errores_consecutivos >= MAX_ERRORES_SERIAL_POLL)
                {
                    {
                        std::lock_guard<std::mutex> lock(mtx_poll);
                        poll.store(false, std::memory_order_release);
                    }
                    signal_error.emit(
                        device_id,
                        "Perdida de comunicacion serial con el validador: " + error_serial);
                    break;
                }
            }
            else
            {
                errores_consecutivos = 0;

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

            std::unique_lock<std::mutex> ulock(mtx_poll);
            cv_poll.wait_for(ulock, stoken, std::chrono::milliseconds(poll_milli), [this]() {
                return !poll.load(std::memory_order_acquire);
            });
        }
        CROW_LOG_INFO << "Polling finalizado limpiamente para " << device_id;
    });
}

bool ValidadorUnit::esperar_pago_fisico()
{
    ultimo_error_pago.clear();
    ultimo_entregado_pesos = 0;

    const auto inicio = std::chrono::steady_clock::now();
    auto ultimo_progreso = inicio;
    int32_t max_cents_vistos = 0;
    int errores_serial_consecutivos = 0;

    while (true)
    {
        const auto ahora = std::chrono::steady_clock::now();
        const bool timeout_inactividad = (ahora - ultimo_progreso) > TIMEOUT_INACTIVIDAD_PAGO;
        const bool timeout_global = (ahora - inicio) > TIMEOUT_GLOBAL_PAGO;

        if (timeout_inactividad || timeout_global)
        {
            const std::string motivo = timeout_inactividad
                                           ? "Timeout por inactividad en entrega fisica"
                                           : "Timeout global alcanzado en entrega fisica";
            CROW_LOG_WARNING << motivo << " de " << device_id
                             << " (ultimo avance reportado: $" << (max_cents_vistos / 100)
                             << "). Deteniendo motor (0x38)...";

            detener_pago();

            // Drenado controlado tras 0x38 hasta que el motor frene y no haya más actividad
            bool termino_con_exito = false;
            int polls_quietos = 0;

            for (int intento = 0; intento < MAX_POLLS_DRENADO_HALT; ++intento)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(150));

                bool hubo_dispensing = false;
                bool evento_final = false;

                {
                    std::lock_guard<std::mutex> lock(mtx_comunicacion);
                    try
                    {
                        auto eventos = rust_unit->hacer_poll();
                        for (const auto &ev : eventos)
                        {
                            if (ev.value_cents > max_cents_vistos)
                            {
                                max_cents_vistos = ev.value_cents;
                                ultimo_entregado_pesos = static_cast<uint32_t>(max_cents_vistos / 100);
                            }

                            if (ev.kind == ffi::EventKind::Dispensing)
                            {
                                hubo_dispensing = true;
                            }
                            else if (ev.kind == ffi::EventKind::Dispensed)
                            {
                                termino_con_exito = true;
                                evento_final = true;
                            }
                            else if (ev.kind == ffi::EventKind::IncompletePayout ||
                                     ev.kind == ffi::EventKind::Timeout ||
                                     ev.kind == ffi::EventKind::Jammed ||
                                     ev.kind == ffi::EventKind::Error)
                            {
                                if (!ev.message.empty())
                                    ultimo_error_pago = std::string(ev.message);
                                evento_final = true;
                            }
                        }
                    }
                    catch (const std::exception &e)
                    {
                        CROW_LOG_ERROR << "Error drenando tras 0x38 en " << device_id << ": " << e.what();
                        break;
                    }
                }

                if (evento_final)
                    break;

                if (hubo_dispensing)
                    polls_quietos = 0;
                else if (++polls_quietos >= POLLS_QUIETOS_TRAS_HALT)
                    break;
            }

            if (termino_con_exito)
                return true;

            if (ultimo_error_pago.empty())
                ultimo_error_pago = motivo + " (motor detenido con 0x38).";

            return false;
        }

        bool exito = false;
        bool terminado = false;

        {
            std::lock_guard<std::mutex> lock(mtx_comunicacion);
            try
            {
                auto eventos = rust_unit->hacer_poll();
                errores_serial_consecutivos = 0;

                for (const auto &ev : eventos)
                {
                    if (ev.kind == ffi::EventKind::Dispensing)
                    {
                        if (ev.value_cents > max_cents_vistos)
                        {
                            max_cents_vistos = ev.value_cents;
                            ultimo_entregado_pesos = static_cast<uint32_t>(max_cents_vistos / 100);
                            ultimo_progreso = std::chrono::steady_clock::now();
                        }
                    }
                    else if (ev.kind == ffi::EventKind::Dispensed)
                    {
                        if (ev.value_cents > max_cents_vistos)
                        {
                            max_cents_vistos = ev.value_cents;
                            ultimo_entregado_pesos = static_cast<uint32_t>(max_cents_vistos / 100);
                        }
                        exito = true;
                        terminado = true;
                    }
                    else if (ev.kind == ffi::EventKind::Timeout ||
                             ev.kind == ffi::EventKind::IncompletePayout ||
                             ev.kind == ffi::EventKind::Jammed ||
                             ev.kind == ffi::EventKind::Error)
                    {
                        if (ev.value_cents > max_cents_vistos)
                        {
                            max_cents_vistos = ev.value_cents;
                            ultimo_entregado_pesos = static_cast<uint32_t>(max_cents_vistos / 100);
                        }
                        ultimo_error_pago = !ev.message.empty()
                                                ? std::string(ev.message)
                                                : "Atasco o pago incompleto reportado por el hardware.";
                        CROW_LOG_WARNING << "Evento de fallo en pago (" << device_id << "): "
                                         << ultimo_error_pago;
                        exito = false;
                        terminado = true;
                    }
                }
            }
            catch (const std::exception &e)
            {
                ++errores_serial_consecutivos;
                CROW_LOG_ERROR << "Error serial esperando pago en " << device_id << " ["
                               << errores_serial_consecutivos << "/" << MAX_ERRORES_SERIAL_PAGO
                               << "]: " << e.what();

                if (errores_serial_consecutivos >= MAX_ERRORES_SERIAL_PAGO)
                {
                    ultimo_error_pago = std::string("Perdida de comunicacion durante entrega: ") + e.what();
                    return false;
                }
            }
        }

        if (terminado)
            return exito;

        std::this_thread::sleep_for(std::chrono::milliseconds(poll_milli));
    }
}

std::vector<ffi::PayoutItem> ValidadorUnit::calcular_cambio(
    uint32_t &cambio_restante,
    const std::vector<ffi::DenomLevel> &niveles,
    bool is_cambio) const
{
    const uint32_t objetivo = cambio_restante;
    if (objetivo == 0)
        return {};

    struct DenomCand
    {
        uint32_t valor_pesos;
        int32_t valor_cents;
        uint32_t stock;
    };

    std::vector<DenomCand> denoms;
    denoms.reserve(niveles.size());

    for (const auto &d : niveles)
    {
        if (d.value_pesos <= 0 || d.stored_in_payout <= 0)
            continue;
        const uint32_t val = static_cast<uint32_t>(d.value_pesos);
        if (is_cambio && val >= objetivo)
            continue;
        denoms.push_back(DenomCand{
            .valor_pesos = val,
            .valor_cents = d.value_cents,
            .stock = static_cast<uint32_t>(d.stored_in_payout),
        });
    }

    if (denoms.empty())
        return {};

    std::sort(denoms.begin(), denoms.end(), [](const DenomCand &a, const DenomCand &b) {
        return a.valor_pesos > b.valor_pesos;
    });

    std::vector<uint64_t> max_desde(denoms.size() + 1, 0);
    for (size_t i = denoms.size(); i-- > 0;)
    {
        max_desde[i] = max_desde[i + 1] +
                       static_cast<uint64_t>(denoms[i].valor_pesos) *
                           static_cast<uint64_t>(denoms[i].stock);
    }

    uint32_t mejor_remanente = objetivo;
    uint32_t mejor_piezas = std::numeric_limits<uint32_t>::max();
    std::vector<uint32_t> mejor_sol(denoms.size(), 0);
    std::vector<uint32_t> actual(denoms.size(), 0);
    uint32_t nodos_visitados = 0;

    auto dfs = [&](auto &&self, size_t idx, uint32_t restante, uint32_t piezas_usadas) -> void
    {
        if (restante < mejor_remanente ||
            (restante == mejor_remanente && piezas_usadas < mejor_piezas))
        {
            mejor_remanente = restante;
            mejor_piezas = piezas_usadas;
            mejor_sol = actual;
        }

        if (idx == denoms.size() || restante == 0 || ++nodos_visitados >= MAX_NODOS_DFS_CAMBIO)
            return;

        const uint64_t min_posible = (static_cast<uint64_t>(restante) > max_desde[idx])
                                         ? (static_cast<uint64_t>(restante) - max_desde[idx])
                                         : 0ULL;
        if (min_posible > static_cast<uint64_t>(mejor_remanente))
            return;

        const uint32_t valor = denoms[idx].valor_pesos;
        const uint32_t max_usar = std::min(restante / valor, denoms[idx].stock);

        for (uint32_t usar = max_usar;; --usar)
        {
            const uint32_t nuevas_piezas = piezas_usadas + usar;
            if (!(mejor_remanente == 0 && nuevas_piezas >= mejor_piezas))
            {
                actual[idx] = usar;
                self(self, idx + 1, restante - usar * valor, nuevas_piezas);
            }
            if (usar == 0 || nodos_visitados >= MAX_NODOS_DFS_CAMBIO)
                break;
        }
        actual[idx] = 0;
    };

    dfs(dfs, 0, objetivo, 0);

    cambio_restante = mejor_remanente;

    std::vector<ffi::PayoutItem> a_dispensar;
    for (size_t i = 0; i < denoms.size(); ++i)
    {
        if (mejor_sol[i] > 0)
        {
            a_dispensar.push_back(ffi::PayoutItem{
                .value_cents = denoms[i].valor_cents,
                .count = static_cast<int32_t>(mejor_sol[i]),
            });
        }
    }
    return a_dispensar;
}

uint32_t ValidadorUnit::iniciar_pago_automatico(uint32_t monto_pesos, bool is_cambio)
{
    detener_polling();

    uint32_t remanente_teorico = monto_pesos;
    const auto niveles_antes = get_nivel_actual();
    const auto piezas = calcular_cambio(remanente_teorico, niveles_antes, is_cambio);

    if (piezas.empty())
    {
        return monto_pesos;
    }

    const bool ok = iniciar_pago_manual(piezas);
    if (ok)
    {
        return remanente_teorico;
    }

    // Si hubo fallo/timeout/atasco, calculamos el remanente real comparando inventario físico
    // antes vs después (ya refrescado dentro de iniciar_pago_manual), con respaldo del contador
    // reportado por el hardware si no hubiera podido leerse el nivel final.
    uint32_t entregado_real = calcular_entregado_por_niveles(niveles_antes, ultimo_cash_level);
    if (entregado_real == 0 && ultimo_entregado_pesos > 0)
    {
        entregado_real = ultimo_entregado_pesos;
    }

    const uint32_t entregado_acotado = std::min(monto_pesos, entregado_real);
    const uint32_t remanente_real = monto_pesos - entregado_acotado;

    CROW_LOG_WARNING << "Pago automatico parcial/fallido en " << device_id
                     << ": solicitado $" << monto_pesos
                     << ", entregado real $" << entregado_acotado
                     << ", remanente real $" << remanente_real;

    return remanente_real;
}

bool ValidadorUnit::iniciar_pago_manual(const std::vector<ffi::PayoutItem> &piezas)
{
    if (piezas.empty())
        return true;

    detener_polling();

    try
    {
        {
            std::lock_guard<std::mutex> lock(mtx_comunicacion);
            rust::Slice<const ffi::PayoutItem> slice(piezas.data(), piezas.size());
            rust_unit->dispensar_piezas(slice);
        }

        const bool ok = esperar_pago_fisico();
        // Siempre actualizamos los niveles reales para registrar salidas parciales si hubo timeout/atasco
        get_nivel_actual();

        if (!ok)
        {
            const std::string msg = !ultimo_error_pago.empty()
                                        ? ultimo_error_pago
                                        : "Atasco o tiempo agotado durante la entrega de efectivo.";
            signal_error.emit(device_id, msg);
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
    try
    {
        rust_unit->aceptar_escrow();
        return true;
    }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << "Error aceptando billete en " << device_id << ": " << e.what();
        return false;
    }
}

bool ValidadorUnit::rechazar_escrow()
{
    std::lock_guard<std::mutex> lock(mtx_comunicacion);
    try
    {
        rust_unit->rechazar_escrow();
        return true;
    }
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
    const bool tiene_piezas =
        std::any_of(cantidades.begin(), cantidades.end(), [](int32_t c) { return c > 0; });
    if (!tiene_piezas)
        return true;

    detener_polling();

    try
    {
        {
            std::lock_guard<std::mutex> lock(mtx_comunicacion);
            rust::Slice<const int32_t> slice(cantidades.data(), cantidades.size());
            rust_unit->dispensar_por_canales(slice);
        }

        const bool ok = esperar_pago_fisico();
        get_nivel_actual();

        if (!ok)
        {
            const std::string msg = !ultimo_error_pago.empty()
                                        ? ultimo_error_pago
                                        : "Atasco o tiempo agotado durante la entrega de efectivo.";
            signal_error.emit(device_id, msg);
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
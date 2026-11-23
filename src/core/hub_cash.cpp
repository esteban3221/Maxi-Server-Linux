#include "core/hub_cash.hpp"

#include <crow/logging.h>
#include <limits>
#include <stdexcept>
#include <unordered_map>

namespace
{
constexpr size_t NO_TX = std::numeric_limits<size_t>::max();
}

#ifdef ESP_PLATFORM
void CashHub::inicializar_hardware()
{
    ESP_LOGI("CashHub", "Iniciando escaneo de IF17 en USB Host...");
    {
        std::lock_guard<std::mutex> lock(m_hub_mutex);
        unidades.clear();
    }

    for (const auto &port : UsbBridge::instance().get_ftdi_ports())
    {
        int32_t ssp_sugerido = ffi::detectar_ssp_en_puerto(port);
        if (ssp_sugerido != -1)
        {
            ESP_LOGI("CashHub", "Puerto %s detectado como %s (SSP %d)",
                     port.c_str(), (ssp_sugerido == 16 ? "COIN" : "BILL"), ssp_sugerido);
            intentar_registrar(port, ssp_sugerido);
        }
        else
        {
            adw_toast_show(("No se pudo registrar validador en " + port).c_str(), 3000);
        }
    }
}
#else
void CashHub::inicializar_hardware()
{
    CROW_LOG_INFO << "Iniciando escaneo nativo de puertos USB...";
    {
        std::lock_guard<std::mutex> lock(m_hub_mutex);
        unidades.clear();
    }

    std::error_code ec;
    fs::directory_iterator dir_it("/dev", ec);
    if (ec)
    {
        CROW_LOG_ERROR << "Error abriendo /dev: " << ec.message();
        return;
    }

    for (const auto &entry : dir_it)
    {
        auto path = entry.path().string();
        if (path.find("/dev/ttyUSB") != std::string::npos)
        {
            // Rust detecta si responde 16 (Monedero) o 0 (Billetero)
            int32_t ssp_sugerido = ffi::detectar_ssp_en_puerto(path);
            if (ssp_sugerido != -1)
            {
                CROW_LOG_INFO << "Puerto " << path << " detectado como Validador tipo "
                              << (ssp_sugerido == 16 ? "COIN" : "BILL")
                              << ". Usando SSP " << ssp_sugerido;
                intentar_registrar(path, ssp_sugerido);
            }
            else
            {
                CROW_LOG_CRITICAL << "No se pudo registrar el validador en " << path;
                Global::System::showNotify("Init System", ("No se pudo registrar el validador en " + path).c_str(), "dialog-error");
            }
        }
    }
}
#endif

bool CashHub::intentar_registrar(const std::string &puerto, int ssp)
{
    auto v = std::make_unique<ValidadorUnit>();

    v->property_conf().puerto = puerto;
    v->property_conf().ssp = ssp;
    v->property_conf().habilita_recolector = false;
    v->property_conf().auto_acepta_credito = false;
    v->property_conf().habilita_salida_credito = false;

    if (v->inicia_conecta())
    {
        ValidadorUnit *raw_v = v.get();

        v->signal_event_received.connect([this, raw_v](const ffi::CashEvent &ev)
        {
            bool enviar_a_cashbox = false;

            if (ev.kind == ffi::EventKind::Credit && ev.device_type == ffi::DeviceType::Bill)
            {
                std::lock_guard<std::mutex> lock(m_billetes_mutex);
                if (m_list_billetes)
                {
                    for (size_t i = 0; i < m_list_billetes->get_n_items(); i++)
                    {
                        auto item = m_list_billetes->get_item(i);
                        if (ev.value_pesos == item->m_denominacion)
                        {
                            if (item->m_cant_recy < item->m_nivel_inmo_max)
                            {
                                ++item->m_cant_recy;
                                if (item->m_cant_recy >= item->m_nivel_inmo_max)
                                {
                                    enviar_a_cashbox = true;
                                }
                            }
                            break;
                        }
                    }
                }
            }

            if (enviar_a_cashbox)
            {
                raw_v->cambiar_ruta(ev.value_pesos, false);
            }

            signal_credito.emit(ev);
        });

        v->signal_error.connect([this](const std::string &device_id, const std::string &error_msg)
        {
            signal_hub_error.emit(device_id, error_msg);
        });

        v->pausar_sesion();
        {
            std::lock_guard<std::mutex> lock(m_hub_mutex);
            unidades.emplace_back(std::move(v));
        }
        return true;
    }

    v->detiene_desconecta();
    return false;
}

std::vector<ffi::RouteRule> CashHub::rutas_default(const ValidadorUnit *val)
{
    if (!val || val->property_device_type() == ffi::DeviceType::Coin)
        return {};

    auto m_list = LevelCash("Level_Bill").get_level_cash();
    std::unordered_map<int32_t, int32_t> max_por_denom;
    if (m_list)
    {
        for (size_t i = 0; i < m_list->get_n_items(); i++)
        {
            auto item = m_list->get_item(i);
            max_por_denom[item->m_denominacion] = item->m_nivel_inmo_max;
        }
    }

    std::vector<ffi::RouteRule> reglas;
    const auto &snapshot_level = val->property_ultimo_cash_level();
    reglas.reserve(snapshot_level.size());

    for (const auto &lvl : snapshot_level)
    {
        bool es_payout = false;
        if (auto it = max_por_denom.find(lvl.value_pesos); it != max_por_denom.end())
        {
            // Estrictamente menor (<): si ya está en el tope o el tope es 0, va a Cashbox.
            es_payout = (it->second > 0) && (lvl.stored_in_payout < it->second);
        }
        else
        {
            CROW_LOG_WARNING << "Denominacion $" << lvl.value_pesos
                             << " sin regla en Level_Bill; enviando a Cashbox por seguridad.";
        }

        reglas.push_back(ffi::RouteRule{
            .value_cents = lvl.value_cents,
            .to_payout = es_payout
        });

        CROW_LOG_DEBUG << "Cuadrando rutas: $" << lvl.value_pesos
                       << " (En recy: " << lvl.stored_in_payout
                       << ", Destino: " << (es_payout ? "Recycler" : "Cashbox") << ")";
    }

    return reglas;
}

std::map<std::string, DeviceSnapshot> CashHub::obten_ultimo_snapshot_level() const
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);
    std::map<std::string, DeviceSnapshot> respuestas;
    for (const auto &u : unidades)
    {
        respuestas[u->property_device_id()] = DeviceSnapshot{
            .type = u->property_device_type(),
            .levels = u->property_ultimo_cash_level()
        };
    }
    return respuestas;
}

void CashHub::inicia_for_all(const Conf &conf, const std::map<std::string, std::vector<ffi::RouteRule>> &set_routes)
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);

    if (is_in_process.exchange(true))
    {
        CROW_LOG_WARNING << "CashHub ya esta en proceso. Ignorando nueva inicializacion.";
        return;
    }

    std::vector<ValidadorUnit *> iniciadas;
    iniciadas.reserve(unidades.size());

    try
    {
        for (auto &u : unidades)
        {
            u->property_conf().habilita_recolector = conf.habilita_recolector;
            u->property_conf().habilita_salida_credito = conf.habilita_salida_credito;
            u->property_conf().auto_acepta_credito = conf.auto_acepta_credito;

            const auto &dev_id = u->property_device_id();
            bool conectado = false;

            if (auto it = set_routes.find(dev_id); it != set_routes.end())
            {
                conectado = u->inicia_conecta(it->second);
            }
            else
            {
                CROW_LOG_INFO << "Usando rutas default para dispositivo: " << dev_id;
                conectado = u->inicia_conecta(rutas_default(u.get()));
            }

            if (!conectado)
            {
                throw std::runtime_error("No se pudo iniciar/conectar el validador " + dev_id);
            }

            iniciadas.push_back(u.get());

            if (u->property_device_type() == ffi::DeviceType::Bill)
            {
                auto nueva_lista = LevelCash("Level_Bill").get_level_cash();
                const auto &snapshot_level = u->property_ultimo_cash_level();

                if (nueva_lista)
                {
                    for (size_t idx = 0; idx < nueva_lista->get_n_items(); idx++)
                    {
                        auto item = nueva_lista->get_item(idx);
                        for (const auto &lvl : snapshot_level)
                        {
                            if (lvl.value_pesos == item->m_denominacion)
                            {
                                item->m_cant_recy = lvl.stored_in_payout;
                                break;
                            }
                        }
                    }
                }

                std::lock_guard<std::mutex> billetes_lock(m_billetes_mutex);
                m_list_billetes = std::move(nueva_lista);
            }
        }
    }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << "Error inicializando CashHub: " << e.what() << ". Ejecutando rollback...";
        for (auto *u : iniciadas)
        {
            try
            {
                u->pausar_sesion();
            }
            catch (const std::exception &ex_rb)
            {
                CROW_LOG_ERROR << "Error durante rollback en " << u->property_device_id() << ": " << ex_rb.what();
            }
        }
        is_in_process.store(false);
        throw;
    }
    catch (...)
    {
        CROW_LOG_ERROR << "Error desconocido inicializando CashHub. Ejecutando rollback...";
        for (auto *u : iniciadas)
        {
            try
            {
                u->detiene_desconecta();
            }
            catch (...)
            {
            }
        }
        is_in_process.store(false);
        throw;
    }
}

void CashHub::detiene_for_all()
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);
    for (auto &u : unidades)
        u->pausar_sesion();
    is_in_process.store(false);
}

void CashHub::inicia_poll_for_all()
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);
    for (auto &u : unidades)
        u->iniciar_polling();
}

void CashHub::detiene_poll_for_all(size_t t_id)
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);

    // Fase 1: capturar snapshot inicial y detener el polling en TODAS las unidades primero
    // para cerrar la admisión lo antes posible sin dejar un validador activo mientras otro consulta niveles.
    std::vector<std::pair<ValidadorUnit *, std::vector<ffi::DenomLevel>>> snapshots_inicio;
    snapshots_inicio.reserve(unidades.size());

    for (auto &u : unidades)
    {
        snapshots_inicio.emplace_back(u.get(), u->property_ultimo_cash_level());
    }

    for (auto &u : unidades)
    {
        u->detener_polling();
    }

    // Fase 2: leer niveles finales y conciliar diferencias por unidad.
    for (auto &[u, snapshot_inicio] : snapshots_inicio)
    {
        auto snapshot_fin = u->get_nivel_actual();

        if (t_id != NO_TX && !snapshot_inicio.empty() && !snapshot_fin.empty())
        {
            detalle.registrar_diferencias(t_id, snapshot_inicio, snapshot_fin);
        }
        else if (t_id != NO_TX)
        {
            CROW_LOG_ERROR << "Snapshot vacio al comparar niveles en unidad " << u->property_device_id();
        }
    }
}

void CashHub::inicia_pago(size_t t_id, size_t monto_pesos, bool is_cambio)
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);

    const uint32_t monto_inicial = static_cast<uint32_t>(monto_pesos);
    uint32_t remanente = monto_inicial;

    for (auto &u : unidades)
    {
        if (u->property_device_type() == ffi::DeviceType::Bill)
        {
            auto snap_ini = u->property_ultimo_cash_level();
            CROW_LOG_INFO << "Intentando pagar con Billetes...";
            remanente = u->iniciar_pago_automatico(remanente, is_cambio);
            auto snap_fin = u->property_ultimo_cash_level();

            detalle.registrar_diferencias(t_id, snap_ini, snap_fin, true);
        }
    }

    if (remanente > 0)
    {
        // Si ya se entregó una parte en billetes, no se debe bloquear en monedas
        // el uso de una moneda igual al remanente (ej. moneda de $10 para remanente $10).
        const bool coin_is_cambio = is_cambio && (remanente == monto_inicial);

        for (auto &u : unidades)
        {
            if (u->property_device_type() == ffi::DeviceType::Coin)
            {
                auto snap_ini = u->property_ultimo_cash_level();
                CROW_LOG_INFO << "Intentando pagar resto con Monedas...";
                remanente = u->iniciar_pago_automatico(remanente, coin_is_cambio);
                auto snap_fin = u->property_ultimo_cash_level();

                detalle.registrar_diferencias(t_id, snap_ini, snap_fin, true);
            }
        }
    }

    if (remanente > 0)
    {
        signal_hub_error.emit("General", "Cambio incompleto. Faltaron: $" + std::to_string(remanente));
    }
}

void CashHub::inicia_pago(size_t t_id, const std::map<std::string, std::vector<ffi::PayoutItem>> &pagos_manuales)
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);

    for (auto &u : unidades)
    {
        const auto &dev_id = u->property_device_id();
        if (auto it = pagos_manuales.find(dev_id); it != pagos_manuales.end())
        {
            auto snap_ini = u->property_ultimo_cash_level();
            u->iniciar_pago_manual(it->second);
            auto snap_fin = u->property_ultimo_cash_level();

            detalle.registrar_diferencias(t_id, snap_ini, snap_fin);
        }
        else
        {
            CROW_LOG_WARNING << "Dispositivo " << dev_id << " omitido en pago manual.";
        }
    }
}

std::vector<ffi::DenomLevel> CashHub::get_nivel_actual_by_id(const std::string &device_id) const
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);
    for (const auto &u : unidades)
    {
        if (u->property_device_id() == device_id)
            return u->get_nivel_actual();
    }
    CROW_LOG_ERROR << "Dispositivo " << device_id << " no encontrado en el Hub";
    return {};
}

bool CashHub::vaciar_dispositivo(const std::string &device_id, bool smart_empty)
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);
    for (auto &u : unidades)
    {
        if (u->property_device_id() == device_id)
            return u->vaciar_reciclador(smart_empty);
    }
    return false;
}

void CashHub::inicia_pago_por_canales(size_t t_id, const std::map<std::string, std::vector<int32_t>> &mapa_canales)
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);

    for (auto &u : unidades)
    {
        const auto &dev_id = u->property_device_id();
        if (auto it = mapa_canales.find(dev_id); it != mapa_canales.end())
        {
            auto snap_ini = u->property_ultimo_cash_level();
            u->iniciar_pago_por_canales(it->second);
            auto snap_fin = u->property_ultimo_cash_level();

            detalle.registrar_diferencias(t_id, snap_ini, snap_fin, true);
        }
        else
        {
            CROW_LOG_WARNING << "Dispositivo " << dev_id << " omitido en pago manual por canales.";
        }
    }
}

bool CashHub::aceptar_escrow(const std::string &device_id)
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);
    for (auto &u : unidades)
        if (u->property_device_id() == device_id)
            return u->aceptar_escrow();
    return false;
}

bool CashHub::rechazar_escrow(const std::string &device_id)
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);
    for (auto &u : unidades)
        if (u->property_device_id() == device_id)
            return u->rechazar_escrow();
    return false;
}

void CashHub::limpiar_cashbox(const std::string &device_id)
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);
    for (auto &u : unidades)
        if (u->property_device_id() == device_id)
            u->limpiar_niveles_cashbox();
}

std::map<std::string, DeviceSnapshot> CashHub::obten_niveles_actuales_for_all()
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);
    std::map<std::string, DeviceSnapshot> respuestas;
    for (auto &u : unidades)
    {
        respuestas[u->property_device_id()] = DeviceSnapshot{
            .type = u->property_device_type(),
            .levels = u->get_nivel_actual()
        };
    }
    return respuestas;
}

void CashHub::reiniciar_for_all()
{
    std::lock_guard<std::mutex> lock(m_hub_mutex);
    for (auto &u : unidades)
        u->reiniciar();

    is_in_process.store(false);
}
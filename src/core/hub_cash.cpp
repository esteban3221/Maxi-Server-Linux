#include "core/hub_cash.hpp"
#include <crow/logging.h>

void CashHub::inicializar_hardware()
{
    CROW_LOG_INFO << "Iniciando escaneo nativo de puertos USB...";
    unidades.clear();

    for (const auto &entry : fs::directory_iterator("/dev"))
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
                Global::System::showNotify("Init System", ("No se pudo registrar el validador en " + path).c_str(), "dialog-error");
            }
        }
    }
}

bool CashHub::intentar_registrar(const std::string &puerto, int ssp)
{
    auto v = std::make_unique<ValidadorUnit>();

    v->property_conf().puerto = puerto;
    v->property_conf().ssp = static_cast<uint32_t>(ssp);
    v->property_conf().habilita_recolector = false;
    v->property_conf().auto_acepta_credito = false;
    v->property_conf().habilita_salida_credito = false;

    if (v->inicia_conecta())
    {
        ValidadorUnit *raw_v = v.get();

        v->signal_event_received.connect([this, raw_v](const ffi::CashEvent &ev)
        {
            if (ev.device_type == ffi::DeviceType::Bill && m_list_billetes)
            {
                for (size_t i = 0; i < m_list_billetes->get_n_items(); i++)
                {
                    auto item = m_list_billetes->get_item(i);
                    if (ev.value_pesos == (item->m_denominacion))
                    {
                        if (++item->m_cant_recy >= item->m_nivel_inmo_max)
                        {
                            raw_v->cambiar_ruta(ev.value_pesos, false);
                        }
                    }
                }
            }
            signal_credito.emit(ev);
        });

        v->signal_error.connect([this](const std::string &device_id, const std::string &error_msg)
        {
            signal_hub_error.emit(device_id, error_msg);
        });

        v->detiene_desconecta();
        unidades.push_back(std::move(v));
        return true;
    }

    v->detiene_desconecta();
    return false;
}

std::vector<ffi::RouteRule> CashHub::rutas_default(const ValidadorUnit *val)
{
    if (val->property_device_type() == ffi::DeviceType::Coin)
        return {};

    std::vector<ffi::RouteRule> reglas;
    auto m_list = std::make_unique<LevelCash>("Level_Bill")->get_level_cash();
    const auto &snapshot_level = val->property_ultimo_cash_level();

    for (size_t i = 0; i < snapshot_level.size() && i < m_list->get_n_items(); i++)
    {
        bool es_payout = (snapshot_level[i].stored_in_payout <= static_cast<int32_t>(m_list->get_item(i)->m_nivel_inmo_max));
        reglas.push_back(ffi::RouteRule{
            .value_cents = snapshot_level[i].value_cents,
            .to_payout = es_payout
        });

        CROW_LOG_DEBUG << "Cuadrando rutas: $" << snapshot_level[i].value_pesos
                       << " (Destino: " << (es_payout ? "Recycler" : "Cashbox") << ")";
    }
    return reglas;
}

std::map<std::string, DeviceSnapshot> CashHub::obten_ultimo_snapshot_level() const
{
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
    if (is_in_process.exchange(true))
    {
        CROW_LOG_WARNING << "CashHub ya esta en proceso. Ignorando nueva inicializacion.";
        return;
    }

    try
    {
        for (auto &u : unidades)
        {
            u->property_conf().habilita_recolector = conf.habilita_recolector;
            u->property_conf().habilita_salida_credito = conf.habilita_salida_credito;
            u->property_conf().auto_acepta_credito = conf.auto_acepta_credito;

            const auto &dev_id = u->property_device_id();
            if (auto it = set_routes.find(dev_id); it != set_routes.end())
            {
                u->inicia_conecta(it->second);
            }
            else
            {
                CROW_LOG_INFO << "Usando rutas default para dispositivo: " << dev_id;
                u->inicia_conecta(rutas_default(u.get()));
            }

            if (u->property_device_type() == ffi::DeviceType::Bill)
            {
                m_list_billetes = std::make_unique<LevelCash>("Level_Bill")->get_level_cash();
                const auto &snapshot_level = u->property_ultimo_cash_level();

                for (size_t idx = 0; idx < m_list_billetes->get_n_items(); idx++)
                {
                    auto item = m_list_billetes->get_item(idx);
                    for (const auto &lvl : snapshot_level)
                    {
                        if (lvl.value_pesos == static_cast<int32_t>(item->m_denominacion))
                        {
                            item->m_cant_recy = lvl.stored_in_payout;
                        }
                    }
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << "Error inicializando CashHub: " << e.what();
        is_in_process.store(false);
        throw;
    }
}

void CashHub::detiene_for_all()
{
    for (auto &u : unidades)
        u->detiene_desconecta();
    is_in_process.store(false);
}

void CashHub::inicia_poll_for_all()
{
    for (auto &u : unidades)
        u->iniciar_polling();
}

void CashHub::detiene_poll_for_all(size_t t_id)
{
    for (auto &u : unidades)
    {
        auto snapshot_inicio = u->property_ultimo_cash_level();
        u->detener_polling();
        auto snapshot_fin = u->get_nivel_actual();

        if (t_id != (-1) && !snapshot_inicio.empty() && !snapshot_fin.empty())
        {
            detalle.registrar_diferencias(t_id, snapshot_inicio, snapshot_fin);
        }
        else if (t_id != (-1))
        {
            CROW_LOG_ERROR << "Snapshot vacio al comparar niveles en unidad " << u->property_device_id();
        }
    }
}

void CashHub::inicia_pago(size_t t_id, size_t monto_pesos, bool is_cambio)
{
    uint32_t remanente = static_cast<uint32_t>(monto_pesos);

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
        for (auto &u : unidades)
        {
            if (u->property_device_type() == ffi::DeviceType::Coin)
            {
                auto snap_ini = u->property_ultimo_cash_level();
                CROW_LOG_INFO << "Intentando pagar resto con Monedas...";
                remanente = u->iniciar_pago_automatico(remanente, is_cambio);
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
    for (const auto &u : unidades)
    {
        if (u->property_device_id() == device_id)
            return u->get_nivel_actual();
    }
    CROW_LOG_ERROR << "Dispositivo " + device_id + " no encontrado en el Hub";
    return {};
}

bool CashHub::vaciar_dispositivo(const std::string &device_id, bool smart_empty)
{
    for (auto &u : unidades)
    {
        if (u->property_device_id() == device_id)
            return u->vaciar_reciclador(smart_empty);
    }
    return false;
}


void CashHub::inicia_pago_por_canales(size_t t_id, const std::map<std::string, std::vector<int32_t>> &mapa_canales)
{
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
    for (auto &u : unidades)
        if (u->property_device_id() == device_id)
            return u->aceptar_escrow();
    return false;
}

bool CashHub::rechazar_escrow(const std::string &device_id)
{
    for (auto &u : unidades)
        if (u->property_device_id() == device_id)
            return u->rechazar_escrow();
    return false;
}

void CashHub::limpiar_cashbox(const std::string &device_id)
{
    for (auto &u : unidades)
        if (u->property_device_id() == device_id)
            u->limpiar_niveles_cashbox();
}

std::map<std::string, DeviceSnapshot> CashHub::obten_niveles_actuales_for_all()
{
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
    for (auto &u : unidades)
    {
        u->reiniciar();
    }
    is_in_process.store(false);
}
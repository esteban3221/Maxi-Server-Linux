#include "model/detalle_movimiento.hpp"
#include <map>
#include <cmath>
#include <cstdint>

void DetalleMovimiento::insertar_detalle_movimiento(uint32_t id_log, const Glib::RefPtr<MDetalleMovimiento> &detalle)
{
    auto &db = Database::getInstance();
    db.sqlite3->command(
        "INSERT INTO detalle_movimientos_dinero (id_log, tipo_movimiento, denominacion, cantidad, creado_en) VALUES (?, ?, ?, ?, ?)",
        id_log,
        detalle->m_tipo_movimiento.c_str(),
        detalle->m_denominacion,
        detalle->m_cantidad,
        detalle->m_creado_en.format_iso8601().c_str());
}

const std::shared_ptr<ResultMap> DetalleMovimiento::get_detalle_movimiento(uint32_t id_log)
{
    auto &db = Database::getInstance();
    return db.sqlite3->command("SELECT * FROM detalle_movimientos_dinero WHERE id_log = ?", id_log);
}

void DetalleMovimiento::registrar_diferencias(
    size_t t_id,
    const std::vector<ffi::DenomLevel> &inicial,
    const std::vector<ffi::DenomLevel> &final_,
    bool solo_salidas)
{
    if (inicial.empty() || final_.empty())
    {
        CROW_LOG_ERROR << "Diferencias abortadas: Snapshot inicial o final vacío.";
        return;
    }

    std::map<uint32_t, int32_t> niveles_inicial;
    for (const auto &item : inicial)
    {
        if (item.value_pesos > 0)
        {
            niveles_inicial[static_cast<uint32_t>(item.value_pesos)] = item.stored_in_payout;
        }
    }

    for (const auto &item : final_)
    {
        if (item.value_pesos <= 0)
            continue;

        uint32_t denom = static_cast<uint32_t>(item.value_pesos);
        int32_t cant_final = item.stored_in_payout;
        int32_t cant_inicial = niveles_inicial.count(denom) ? niveles_inicial[denom] : 0;
        int32_t delta = cant_final - cant_inicial;

        if (delta == 0 || (solo_salidas && delta > 0))
            continue;

        std::string tipo = (delta > 0) ? "entrada" : "salida";
        uint32_t piezas = static_cast<uint32_t>(std::abs(delta));

        CROW_LOG_INFO << "Movimiento detectado (" << tipo << "): " << piezas
                      << " piezas de $" << denom << " (ID: " << t_id << ")";

        this->insertar_detalle_movimiento(
            static_cast<uint32_t>(t_id),
            MDetalleMovimiento::create(
                0, static_cast<uint32_t>(t_id), tipo, denom, piezas,
                Glib::DateTime::create_now_local()));
    }
}
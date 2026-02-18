#include "model/detalle_movimiento.hpp"

DetalleMovimiento::DetalleMovimiento(/* args */)
{
}

DetalleMovimiento::~DetalleMovimiento()
{
}

void DetalleMovimiento::insertar_detalle_movimiento(uint32_t id_log, const Glib::RefPtr<Gio::ListStore<MDetalleMovimiento>> &detalle)
{
    auto &db = Database::getInstance();
    for (guint i = 0; i < detalle->get_n_items(); ++i)
        db.sqlite3->command("INSERT INTO detalle_movimientos_dinero (id_log, tipo_movimiento, denominacion, cantidad, creado_en) VALUES (?, ?, ?, ?, ?)",
                            id_log,
                            detalle->get_item(i)->m_tipo_movimiento,
                            detalle->get_item(i)->m_denominacion,
                            detalle->get_item(i)->m_cantidad,
                            detalle->get_item(i)->m_creado_en.format_iso8601());
}

const std::shared_ptr<ResultMap> DetalleMovimiento::get_detalle_movimiento(uint32_t id_log)
{
    auto &db = Database::getInstance();
    return db.sqlite3->command("SELECT * FROM detalle_movimientos_dinero WHERE id_log = ?", id_log);
}

#include <map>
#include <cstdint>
#include <giomm.h>

/**
 * Calcula las diferencias entre dos estados de niveles de cash.
 * 
 * @param inicial Lista inicial (snapshot antes del movimiento)
 * @param final   Lista final (snapshot después del movimiento)
 * @return std::map<denominacion, delta> 
 *         delta > 0 → entradas (billetes/monedas agregados)
 *         delta < 0 → salidas (billetes/monedas retirados)
 *         Si no existe en inicial/final, se considera 0
 */
/**
 * Calcula diferencias y devuelve dos mapas separados:
 * - entradas: denominacion → cantidad positiva agregada
 * - salidas:  denominacion → cantidad positiva retirada
 */
/**
 * Calcula diferencias entre snapshot inicial y final.
 * 
 * delta > 0  → Entrada (cantidad positiva)
 * delta < 0  → Salida  (cantidad negativa)  ← como tú quieres
 * 
 * Devuelve exactamente el mismo tipo que ya estás usando.
 */
std::map<uint32_t, int32_t> DetalleMovimiento::calcular_diferencias_niveles(
    const Glib::RefPtr<Gio::ListStore<MLevelCash>>& inicial,   // ← ANTES del movimiento
    const Glib::RefPtr<Gio::ListStore<MLevelCash>>& final_)    // ← DESPUÉS del movimiento
{
    std::map<uint32_t, int32_t> diferencias;

    std::set<uint32_t> todas_denominaciones;

    // Recolectar todas las denominaciones que existen en cualquiera de los dos estados
    for (guint i = 0; i < inicial->get_n_items(); ++i)
        todas_denominaciones.insert(inicial->get_item(i)->m_denominacion);

    for (guint i = 0; i < final_->get_n_items(); ++i)
        todas_denominaciones.insert(final_->get_item(i)->m_denominacion);

    for (uint32_t denom : todas_denominaciones)
    {
        uint32_t cant_inicial = 0;
        uint32_t cant_final   = 0;

        // Buscar en inicial
        for (guint j = 0; j < inicial->get_n_items(); ++j)
        {
            auto item = inicial->get_item(j);
            if (item->m_denominacion == denom)
            {
                cant_inicial = item->m_cant_recy;
                break;
            }
        }

        // Buscar en final
        for (guint j = 0; j < final_->get_n_items(); ++j)
        {
            auto item = final_->get_item(j);
            if (item->m_denominacion == denom)
            {
                cant_final = item->m_cant_recy;
                break;
            }
        }

        int32_t delta = static_cast<int32_t>(cant_final) - static_cast<int32_t>(cant_inicial);

        if (delta != 0)
            diferencias[denom] = delta;
    }

    return diferencias;
}
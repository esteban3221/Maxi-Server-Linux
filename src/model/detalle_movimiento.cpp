#include "model/detalle_movimiento.hpp"

DetalleMovimiento::DetalleMovimiento(/* args */)
{
}

DetalleMovimiento::~DetalleMovimiento()
{
}

void DetalleMovimiento::insertar_detalle_movimiento(uint32_t id_log, const Glib::RefPtr<MDetalleMovimiento> &detalle)
{
    auto &db = Database::getInstance();
    db.sqlite3->command("INSERT INTO detalle_movimientos_dinero (id_log, tipo_movimiento, denominacion, cantidad, creado_en) VALUES (?, ?, ?, ?, ?)",
                        id_log,
                        detalle->m_tipo_movimiento,
                        detalle->m_denominacion,
                        detalle->m_cantidad,
                        detalle->m_creado_en.format_iso8601());
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
 *
 * Calcula diferencias y devuelve dos mapas separados:
 * - entradas: denominacion → cantidad positiva agregada
 * - salidas:  denominacion → cantidad positiva retirada
 * Calcula diferencias entre snapshot inicial y final.
 *
 * delta > 0  → Entrada (cantidad positiva)
 * delta < 0  → Salida  (cantidad negativa)
 *
 *
 */
void DetalleMovimiento::registrar_diferencias_salida(size_t t_id, const crow::json::rvalue &inicial, const crow::json::rvalue &final_)
{
    // Mapa para indexar el estado inicial: [Denominación -> Cantidad en Payout]
    std::map<uint32_t, int32_t> niveles_inicial;

    for (auto &item : inicial)
    {
        if (item.has("value") && item.has("storedInPayout"))
        {
            uint32_t denom = item["value"].u() / 100;
            niveles_inicial[denom] = item["storedInPayout"].i();
        }
    }

    // Comparar con el estado final
    for (auto &item : final_)
    {
        if (item.has("value") && item.has("storedInPayout"))
        {
            uint32_t denom = item["value"].u() / 100;
            int32_t cant_final = item["storedInPayout"].i();
            
            // Si la denominación no estaba en el snapshot inicial, asumimos 0
            int32_t cant_inicial = niveles_inicial.count(denom) ? niveles_inicial[denom] : 0;
            
            // Diferencia: Final - Inicial. 
            // Si es -5, significa que el nivel bajó (salieron 5 unidades)
            int32_t delta = cant_final - cant_inicial;

            if (delta < 0)
            {
                uint32_t piezas_salieron = std::abs(delta);
                
                CROW_LOG_DEBUG << "Registrando salida confirmada: " << piezas_salieron 
                               << " piezas de $" << denom << " (ID Movimiento: " << t_id << ")";

                this->insertar_detalle_movimiento(
                    t_id,
                    MDetalleMovimiento::create(
                        0,        // ID autoincremental
                        t_id,     // Relación con el log principal
                        "salida", // Etiqueta de egreso
                        denom,
                        piezas_salieron,
                        Glib::DateTime::create_now_local()
                    )
                );
            }
        }
    }
}
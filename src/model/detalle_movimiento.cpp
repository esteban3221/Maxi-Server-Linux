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

#include <map>
#include <cstdint>
#include <giomm.h>


/**
 * Calcula diferencias entre snapshot inicial y final y registra tanto
 * entradas (delta > 0) como salidas (delta < 0).
 */
void DetalleMovimiento::registrar_diferencias(size_t t_id, const crow::json::rvalue &inicial, const crow::json::rvalue &final_)
{
    // Si el load falló o no son listas, abortamos de forma segura
    if (!inicial || !final_ || inicial.t() != crow::json::type::List || final_.t() != crow::json::type::List)
    {
        CROW_LOG_ERROR << "Diferencias abortadas: Uno de los parámetros no es una lista válida de niveles.";
        return;
    }

    std::map<uint32_t, int32_t> niveles_inicial;

    for (auto &item : inicial)
    {
        if (item.t() == crow::json::type::Object && item.has("value") && item.has("storedInPayout"))
        {
            uint32_t denom = item["value"].u() / 100;
            niveles_inicial[denom] = (int32_t)item["storedInPayout"].i();
        }
    }

    for (auto &item : final_)
    {
        if (item.t() == crow::json::type::Object && item.has("value") && item.has("storedInPayout"))
        {
            uint32_t denom = item["value"].u() / 100;
            int32_t cant_final = (int32_t)item["storedInPayout"].i();
            int32_t cant_inicial = niveles_inicial.count(denom) ? niveles_inicial[denom] : 0;
            int32_t delta = cant_final - cant_inicial;

            if (delta != 0)
            {
                std::string tipo = (delta > 0) ? "entrada" : "salida";
                uint32_t piezas = std::abs(delta);
                
                CROW_LOG_INFO << "Movimiento detectado (" << tipo << "): " << piezas 
                               << " piezas de $" << denom << " (ID: " << t_id << ")";

                this->insertar_detalle_movimiento(
                    t_id,
                    MDetalleMovimiento::create(
                        0, t_id, tipo, denom, piezas,
                        Glib::DateTime::create_now_local()
                    )
                );
            }
        }
    }
}
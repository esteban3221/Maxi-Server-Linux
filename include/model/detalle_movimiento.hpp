#pragma once
#include <iostream>
#include <glibmm.h>
#include <giomm.h>

#include "coneccion.hpp"
#include "model/level_cash.hpp"
// R"(
//                 CREATE TABLE detalle_movimientos_dinero (
//                 id                INTEGER PRIMARY KEY AUTOINCREMENT,
//                 id_log            INTEGER NOT NULL,
//                 tipo_movimiento   TEXT NOT NULL CHECK(tipo_movimiento IN ('entrada', 'salida')),
//                 denominacion      INTEGER NOT NULL,
//                 cantidad          INTEGER NOT NULL DEFAULT 0,
//                 creado_en         DATETIME DEFAULT (datetime('now','localtime')),

//                 FOREIGN KEY (id_log) REFERENCES log(id),

//             );)

class MDetalleMovimiento : public Glib::Object
{
public:
    uint32_t m_id;
    uint32_t m_id_log;
    Glib::ustring m_tipo_movimiento;
    uint32_t m_denominacion;
    uint32_t m_cantidad;
    Glib::DateTime m_creado_en;

    static Glib::RefPtr<MDetalleMovimiento> create(uint32_t id, uint32_t id_log, const Glib::ustring &tipo_movimiento, uint32_t denominacion, uint32_t cantidad, const Glib::DateTime &creado_en = Glib::DateTime::create_now_local())
    {
        return Glib::make_refptr_for_instance<MDetalleMovimiento>(new MDetalleMovimiento(id, id_log, tipo_movimiento, denominacion, cantidad, creado_en));
    }

protected:
    MDetalleMovimiento(uint32_t id, uint32_t id_log, const Glib::ustring &tipo_movimiento, uint32_t denominacion, uint32_t cantidad, const Glib::DateTime &creado_en)
        : m_id(id),
          m_id_log(id_log),
          m_tipo_movimiento(tipo_movimiento),
          m_denominacion(denominacion),
          m_cantidad(cantidad),
          m_creado_en(creado_en)
    {
    }
};

class DetalleMovimiento
{
private:
    /* data */
public:
    void insertar_detalle_movimiento(uint32_t id_log, const Glib::RefPtr<Gio::ListStore<MDetalleMovimiento>> &detalle);
    const std::shared_ptr<ResultMap> get_detalle_movimiento(uint32_t id_log);
    DetalleMovimiento(/* args */);
    ~DetalleMovimiento();

    static std::map<uint32_t, int32_t> calcular_diferencias_niveles(
    const Glib::RefPtr<Gio::ListStore<MLevelCash>>& inicial,
    const Glib::RefPtr<Gio::ListStore<MLevelCash>>& final_);
};
#pragma once
#include <iostream>
#include <vector>
#include <crow.h>
#include <glibmm.h>
#include <giomm.h>

#include "coneccion.hpp"
#include "model/level_cash.hpp"
#include "essp_driver/src/lib.rs.h"

class MDetalleMovimiento : public Glib::Object
{
public:
    uint32_t m_id;
    uint32_t m_id_log;
    Glib::ustring m_tipo_movimiento;
    uint32_t m_denominacion;
    uint32_t m_cantidad;
    Glib::DateTime m_creado_en;

    static Glib::RefPtr<MDetalleMovimiento> create(
        uint32_t id,
        uint32_t id_log,
        const Glib::ustring &tipo_movimiento,
        uint32_t denominacion,
        uint32_t cantidad,
        const Glib::DateTime &creado_en = Glib::DateTime::create_now_local())
    {
        return Glib::make_refptr_for_instance<MDetalleMovimiento>(
            new MDetalleMovimiento(id, id_log, tipo_movimiento, denominacion, cantidad, creado_en));
    }

protected:
    MDetalleMovimiento(
        uint32_t id,
        uint32_t id_log,
        const Glib::ustring &tipo_movimiento,
        uint32_t denominacion,
        uint32_t cantidad,
        const Glib::DateTime &creado_en)
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
public:
    DetalleMovimiento() = default;
    ~DetalleMovimiento() = default;

    void insertar_detalle_movimiento(uint32_t id_log, const Glib::RefPtr<MDetalleMovimiento> &detalle);
    const std::shared_ptr<ResultMap> get_detalle_movimiento(uint32_t id_log);

    uint32_t registrar_diferencias(
        size_t t_id,
        const std::vector<ffi::DenomLevel> &inicial,
        const std::vector<ffi::DenomLevel> &final_);
};
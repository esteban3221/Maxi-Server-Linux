#pragma once
#include <vector>
#include <iostream>
#include <functional>
#include <glibmm.h>
#include <giomm.h>
#include "coneccion.hpp"

class MLogHistorial : public Glib::Object
{
public:
    size_t m_id;
    int m_original_id;
    int m_old_id_user;
    Glib::ustring m_old_tipo;
    int m_old_ingreso;
    int m_old_cambio;
    int m_old_total;
    Glib::ustring m_old_estatus;
    Glib::DateTime m_old_fecha;
    size_t m_new_id_user;
    Glib::ustring m_new_tipo;
    int m_new_ingreso;
    int m_new_cambio;
    int m_new_total;
    Glib::ustring m_new_estatus;
    Glib::DateTime m_new_fecha;
    Glib::DateTime m_fecha_modificacion;
    Glib::ustring m_usuario_operacion;
    Glib::ustring m_tipo_cambio;

    static Glib::RefPtr<MLogHistorial> create(size_t id,
                                                int original_id,
                                                int old_id_user,
                                                const Glib::ustring &old_tipo,
                                                int old_ingreso,
                                                int old_cambio,
                                                int old_total,
                                                const Glib::ustring &old_estatus,
                                                const Glib::DateTime &old_fecha,
                                                size_t new_id_user,
                                                const Glib::ustring &new_tipo,
                                                int new_ingreso,
                                                int new_cambio,
                                                int new_total,
                                                const Glib::ustring &new_estatus,
                                                const Glib::DateTime &new_fecha,
                                                const Glib::DateTime &fecha_modificacion,
                                                const Glib::ustring &usuario_operacion,
                                                const Glib::ustring &tipo_cambio)
    {
        return Glib::make_refptr_for_instance<MLogHistorial>
        (new MLogHistorial
            (
                id,
                original_id,
                old_id_user,
                old_tipo,
                old_ingreso,
                old_cambio,
                old_total,
                old_estatus,
                old_fecha,
                new_id_user,
                new_tipo,
                new_ingreso,
                new_cambio,
                new_total,
                new_estatus,
                new_fecha,
                fecha_modificacion,
                usuario_operacion,
                tipo_cambio
            )
        );
    }

protected:
    MLogHistorial(size_t id,
                    int original_id,
                    int old_id_user,
                    const Glib::ustring &old_tipo,
                    int old_ingreso,
                    int old_cambio,
                    int old_total,
                    const Glib::ustring &old_estatus,
                    const Glib::DateTime &old_fecha,
                    size_t new_id_user,
                    const Glib::ustring &new_tipo,
                    int new_ingreso,
                    int new_cambio,
                    int new_total,
                    const Glib::ustring &new_estatus,
                    const Glib::DateTime &new_fecha,
                    const Glib::DateTime &fecha_modificacion,
                    const Glib::ustring &usuario_operacion,
                    const Glib::ustring &tipo_cambio)
        : m_id(id),
          m_original_id(original_id),
          m_old_id_user(old_id_user),
          m_old_tipo(old_tipo),
          m_old_ingreso(old_ingreso),
          m_old_cambio(old_cambio),
          m_old_total(old_total),
          m_old_estatus(old_estatus),
          m_old_fecha(old_fecha),
          m_new_id_user(new_id_user),
          m_new_tipo(new_tipo),
          m_new_ingreso(new_ingreso),
          m_new_cambio(new_cambio),
          m_new_total(new_total),
          m_new_estatus(new_estatus),
          m_new_fecha(new_fecha),
          m_fecha_modificacion(fecha_modificacion),
          m_usuario_operacion(usuario_operacion),
          m_tipo_cambio(tipo_cambio)
    {
    }
};

class LogHistorial
{
private:
    /* data */
public:
    LogHistorial(/* args */);
    ~LogHistorial();

    Glib::RefPtr<Gio::ListStore<MLogHistorial>> get_log_historial();
    void insert_log_historial(const MLogHistorial &log_historial);
};

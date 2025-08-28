#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <glibmm.h>
#include <giomm.h>
#include <iostream>
#include "coneccion.hpp"

class MLog : public Glib::Object
{
public:
    size_t m_id;
    size_t m_id_user;
    Glib::ustring m_tipo;
    int m_ingreso;
    int m_cambio;
    int m_total;
    Glib::ustring m_estatus;
    Glib::DateTime m_fecha;

    static Glib::RefPtr<MLog> create(size_t id,
                                      size_t id_user,
                                      const Glib::ustring &tipo,
                                      int ingreso,
                                      int cambio,
                                      int total,
                                      const Glib::ustring &estatus,
                                      Glib::DateTime fecha)
    {
        return Glib::make_refptr_for_instance<MLog>(new MLog(id, id_user, tipo, ingreso, cambio, total, estatus, fecha));
    }

protected:
    MLog(size_t id,
          size_t id_user,
          const Glib::ustring &tipo,
          int ingreso,
          int cambio,
          int total,
          const Glib::ustring &estatus,
          const Glib::DateTime &fecha)
        : m_id(id),
          m_id_user(id_user),
          m_tipo(tipo),
          m_ingreso(ingreso),
          m_cambio(cambio),
          m_total(total),
          m_estatus(estatus),
          m_fecha(fecha)
    {
    }
};

class Log
{
private:
public:
    Log(/* args */);
    ~Log();

    Glib::RefPtr<Gio::ListStore<MLog>> get_log(const std::string &tipo = "",const std::string &f_ini = "",const std::string &f_fin = "", int paginacion = 0);
    const std::shared_ptr<ResultMap> get_corte(int id_user, const std::string &tipo);
    size_t insert_log(const Glib::RefPtr<MLog> &list);
    void update_log(const Glib::RefPtr<MLog> &list);
    size_t tam_row;
    // void imprime_log();
};

#pragma once
#include <iostream>
#include <glibmm.h>
#include <giomm.h>

#include "coneccion.hpp"

class MConfiguracion : public Glib::Object
{
public:
    uint32_t m_id;
    Glib::ustring m_descripcion;
    Glib::ustring m_valor;

    static Glib::RefPtr<MConfiguracion> create(uint32_t id, const Glib::ustring &descripcion, const Glib::ustring &valor)
    {
        return Glib::make_refptr_for_instance<MConfiguracion>(new MConfiguracion(id, descripcion, valor));
    }

protected:
    MConfiguracion(uint32_t id, const Glib::ustring &descripcion, const Glib::ustring &valor)
        : m_id(id),
          m_descripcion(descripcion),
          m_valor(valor)
    {
    }
};

class Configuracion
{
private:
    /* data */
public:
    Configuracion(/* args */);
    ~Configuracion();

    Glib::RefPtr<Gio::ListStore<MConfiguracion>> get_conf_data(int inicio, int fin);
    void update_conf(const Glib::RefPtr<MConfiguracion> &conf);
};

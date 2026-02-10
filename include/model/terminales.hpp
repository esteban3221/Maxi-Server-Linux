#pragma once
#include <iostream>
#include <glibmm.h>
#include <giomm.h>

#include "coneccion.hpp"

class MTerminales : public Glib::Object
{
public:
    Glib::ustring m_id;
    Glib::ustring m_tipo;
    Glib::ustring m_alias;
    Glib::ustring m_modo;
    bool m_predeterminado;
    Glib::ustring m_descripcion;
    Glib::DateTime m_fecha_creado;

    static Glib::RefPtr<MTerminales> create(const Glib::ustring &id,
                                            const Glib::ustring &tipo,
                                            const Glib::ustring &alias,
                                            const Glib::ustring &modo,
                                            bool predeterminado,
                                            const Glib::ustring &descripcion,
                                            const Glib::DateTime &fecha_creado)
    {
        return Glib::make_refptr_for_instance<MTerminales>(new MTerminales(id, tipo, alias, modo, predeterminado, descripcion, fecha_creado));
    }

protected:
    MTerminales(const Glib::ustring &id,
                const Glib::ustring &tipo,
                const Glib::ustring &alias,
                const Glib::ustring &modo,
                bool predeterminado,
                const Glib::ustring &descripcion,
                const Glib::DateTime &fecha_creado)
        : m_id(id),
          m_tipo(tipo),
          m_alias(alias),
          m_modo(modo),
          m_predeterminado(predeterminado),
          m_descripcion(descripcion),
          m_fecha_creado(fecha_creado)
    {
    }
};

class OTerminales
{
private:
    /* data */
public:
    OTerminales(/* args */);
    ~OTerminales();

    void inserta(const Glib::RefPtr<MTerminales> &terminal);
    void edita(const Glib::RefPtr<MTerminales> &terminal);
    void elimina(const Glib::ustring &id);

    Glib::RefPtr<MTerminales> get_by_id(const Glib::ustring &id);
    std::shared_ptr<ResultMap> get_all();
};

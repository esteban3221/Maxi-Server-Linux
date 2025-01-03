#pragma once
#include <iostream>
#include <functional>
#include <glibmm.h>
#include <giomm.h>

#include "coneccion.hpp"

class MUsuarioHistorial : public Glib::Object
{
public:
    size_t m_id;
    size_t m_original_id;
    Glib::ustring m_old_username;
    Glib::ustring m_old_password;
    Glib::ustring m_new_username;
    Glib::ustring m_new_password;
    Glib::DateTime m_fecha_modificacion;
    Glib::ustring m_usuario_operacion;
    Glib::ustring m_tipo_cambio;

    static Glib::RefPtr<MUsuarioHistorial> create(
        size_t id,
        size_t original_id,
        const Glib::ustring &old_username,
        const Glib::ustring &old_password,
        const Glib::ustring &new_username,
        const Glib::ustring &new_password,
        const Glib::DateTime &fecha_modificacion,
        const Glib::ustring &usuario_operacion,
        const Glib::ustring &tipo_cambio)
    {
        return Glib::make_refptr_for_instance<MUsuarioHistorial>(new MUsuarioHistorial(
            id,
            original_id,
            old_username,
            old_password,
            new_username,
            new_password,
            fecha_modificacion,
            usuario_operacion,
            tipo_cambio));
    }

protected:
    MUsuarioHistorial(
        size_t id,
        size_t original_id,
        const Glib::ustring &old_username,
        const Glib::ustring &old_password,
        const Glib::ustring &new_username,
        const Glib::ustring &new_password,
        const Glib::DateTime &fecha_modificacion,
        const Glib::ustring &usuario_operacion,
        const Glib::ustring &tipo_cambio)
        : m_id(id),
          m_original_id(original_id),
          m_old_username(old_username),
          m_old_password(old_password),
          m_new_username(new_username),
          m_new_password(new_password),
          m_fecha_modificacion(fecha_modificacion),
          m_usuario_operacion(usuario_operacion),
          m_tipo_cambio(tipo_cambio)
    {
    }
};

class UsuarioHistorial
{
private:
    /* data */
public:
    UsuarioHistorial(/* args */);
    ~UsuarioHistorial();

    Glib::RefPtr<Gio::ListStore<MUsuarioHistorial>> get_usuario_historial();
};

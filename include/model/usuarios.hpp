#pragma once
#include <iostream>
#include <functional>
#include <glibmm.h>
#include <giomm.h>
#include <memory>

#include "coneccion.hpp"

class MUsuarios : public Glib::Object
{
public:
    size_t m_id;
    Glib::ustring m_usuario;
    Glib::ustring m_passsword;

    static Glib::RefPtr<MUsuarios> create(size_t id, const Glib::ustring &usuario, const Glib::ustring &password)
    {
        return Glib::make_refptr_for_instance<MUsuarios>(new MUsuarios(id, usuario, password));
    }

protected:
    MUsuarios(u_int16_t id, const Glib::ustring &usuario, const Glib::ustring &password)
        : m_id(id), m_usuario(usuario), m_passsword(password)
    {
    }
};

class Usuarios
{
private:
    /* data */
public:
    Usuarios(/* args */);
    ~Usuarios();

    Glib::RefPtr<Gio::ListStore<MUsuarios>> get_usuarios();

    /// @brief Verifica si existe un usuario a travez de su contraseña
    /// @param passw
    /// @return Nombre de usuario si es que existe, Si no vacio ""
    const std::string &existe_usuario(const std::string &) const;

    size_t insert_usuario(const Glib::RefPtr<MUsuarios> &usuario);
    void update_usuario(const Glib::RefPtr<MUsuarios> &usuario);
    void delete_usuario(const Glib::RefPtr<MUsuarios> &usuario);
};
#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include <glibmm.h>
#include <giomm.h>

#include "coneccion.hpp"

class MRoles : public Glib::Object
{
public:
    size_t m_id;
    Glib::ustring m_rol;

    static Glib::RefPtr<MRoles> create(size_t id,const Glib::ustring &rol)
    {
        return Glib::make_refptr_for_instance<MRoles>(new MRoles(id, rol));
    }

protected:
    MRoles(size_t id,const Glib::ustring &rol)
        : m_id(id), m_rol(rol)
    {
    }
};

class Roles
{
private:
    /* data */
public:
    Roles(/* args */);
    ~Roles();
    Glib::RefPtr<Gio::ListStore<MRoles>> get_roles();
};
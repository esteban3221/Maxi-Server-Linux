#pragma once
#include <iostream>
#include <functional>
#include <glibmm.h>
#include <giomm.h>

#include "coneccion.hpp"


class MPagoPendiente : public Glib::Object
{
public:
    size_t m_id;
    int m_id_log;
    int m_remanente;
    Glib::ustring m_status;

        static Glib::RefPtr<MPagoPendiente> create(size_t id, int id_log, int remanente, const Glib::ustring & status)
    {
        return Glib::make_refptr_for_instance<MPagoPendiente>(new MPagoPendiente(id, id_log, remanente, status));
    }

protected:
    MPagoPendiente(size_t id, int id_log, int remanente, const Glib::ustring & status)
        : m_id(id), m_id_log(id_log), m_remanente(remanente), m_status(status)
    {
    }

};



class PagoPendiente
{
private:
    /* data */
public:
    PagoPendiente(/* args */);
    ~PagoPendiente();
    Glib::RefPtr<Gio::ListStore<MPagoPendiente>> get_log_historial();
    void update_log_historial(const MPagoPendiente &list);
};
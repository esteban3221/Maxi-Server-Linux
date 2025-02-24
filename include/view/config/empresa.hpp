#pragma once
#include <gtkmm.h>

class VEmpresa : public Gtk::Box
{
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    Gtk::Entry *list_ety_datos[5]{nullptr};

public:
    VEmpresa(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VEmpresa();
};
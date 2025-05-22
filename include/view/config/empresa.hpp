#pragma once
#include <gtkmm.h>

class VEmpresa : public Gtk::Box
{
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    Gtk::Entry *v_list_ety_datos[5]{nullptr};

public:
    VEmpresa(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VEmpresa();
};

namespace View
{
   extern const char *ui_empresa;
}

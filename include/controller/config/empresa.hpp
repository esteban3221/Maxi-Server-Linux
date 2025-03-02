#pragma once
#include "view/config/empresa.hpp"
#include "configuracion.hpp"

class Empresa : public VEmpresa
{
private:
    void init_data();
    void actualiza_datos();
public:
    Empresa(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Empresa();
};
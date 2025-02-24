#pragma once
#include "view/config/general.hpp"
#include "configuracion.hpp"

class General : public VGeneral
{
private:
    /* data */
    void carga_estado_inicial();
public:
    General(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~General();
};

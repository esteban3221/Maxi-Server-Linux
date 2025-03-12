#pragma once
#include "view/base.nip.hpp"
#include "global.hpp"
#include "configuracion.hpp"

class NuevoNip : public VBaseNip
{
private:
    void on_btn_nip_enter() override;
    void on_btn_nip_back() override;

    void borra_cache();

    Glib::ustring nip;
public:
    NuevoNip(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~NuevoNip();
};
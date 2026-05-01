#pragma once
#include <gtkmm.h>

class VBaseNip : public Gtk::Box
{
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    Gtk::Button *v_btn_nip_back = nullptr;
    Gtk::PasswordEntry *v_ety_pin = nullptr;
    Gtk::SpinButton *v_spn_monto = nullptr;
    Gtk::Editable *v_active_editable = nullptr;

    // interfaz
    virtual void on_btn_nip_enter() = 0;
    virtual void on_btn_nip_back() = 0;

public:
    VBaseNip(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VBaseNip();

    void set_modo_monto(bool es_monto);
};

namespace View
{
   extern const char *ui_nip;
}

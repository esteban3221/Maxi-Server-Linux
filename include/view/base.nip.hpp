#pragma once
#include <gtkmm.h>

class VBaseNip : public Gtk::Box
{
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    Gtk::Box *v_box_nip = nullptr;
    Gtk::Button *v_botonera_nip[10]{nullptr};
    Gtk::Button *v_btn_nip_del = nullptr;
    Gtk::Button *v_btn_nip_enter = nullptr;
    Gtk::Button *v_btn_nip_back = nullptr;
    Gtk::PasswordEntry *v_ety_pin = nullptr;

    // interfaz
    virtual void on_btn_nip_enter() = 0;
    virtual void on_btn_nip_back() = 0;

public:
    VBaseNip(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VBaseNip();
};

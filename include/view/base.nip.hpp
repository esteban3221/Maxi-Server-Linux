#pragma once
#include <gtkmm.h>

class VBaseNip : public Gtk::Box
{
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    Gtk::Box *box_nip = nullptr;
    Gtk::Button *botonera_nip[10]{nullptr};
    Gtk::Button *btn_nip_del = nullptr;
    Gtk::Button *btn_nip_enter = nullptr;
    Gtk::Button *btn_nip_back = nullptr;
    Gtk::PasswordEntry *ety_pin = nullptr;

    // interfaz
    virtual void on_btn_nip_enter() = 0;
    virtual void on_btn_nip_back() = 0;

public:
    VBaseNip(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VBaseNip();
};

VBaseNip::VBaseNip(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                            Gtk::Box(cobject)
{
    this->box_nip = m_builder->get_widget<Gtk::Box>("box_nip");
    this->btn_nip_back = m_builder->get_widget<Gtk::Button>("btn_nip_back");
    this->ety_pin = m_builder->get_widget<Gtk::PasswordEntry>("ety_pin");
    this->btn_nip_del = m_builder->get_widget<Gtk::Button>("btn_nip_del");
    this->btn_nip_enter = m_builder->get_widget<Gtk::Button>("btn_nip_enter");

    for (size_t i = 0; i < 10; i++)
        this->botonera_nip[i] = m_builder->get_widget<Gtk::Button>("btn_nip_" + std::to_string(i));
}

VBaseNip::~VBaseNip()
{
}

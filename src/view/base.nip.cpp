#include "view/base.nip.hpp"

VBaseNip::VBaseNip(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                            Gtk::Box(cobject)
{
    this->v_btn_nip_back = m_builder->get_widget<Gtk::Button>("btn_nip_back");
    this->v_ety_pin = m_builder->get_widget<Gtk::PasswordEntry>("ety_pin");
    this->v_btn_nip_del = m_builder->get_widget<Gtk::Button>("btn_nip_del");
    this->v_btn_nip_enter = m_builder->get_widget<Gtk::Button>("btn_nip_enter");

    for (size_t i = 0; i < 10; i++)
        this->v_botonera_nip[i] = m_builder->get_widget<Gtk::Button>("btn_nip_" + std::to_string(i));

    for (int i = 0; i < 10; i++)
        this->v_botonera_nip[i]->signal_clicked().connect([this, i](){ v_ety_pin->set_text(v_ety_pin->get_text() + std::to_string(i)); });

    v_btn_nip_back->signal_clicked().connect(sigc::mem_fun(*this, &VBaseNip::on_btn_nip_back));
    v_btn_nip_del->signal_clicked().connect([this](){v_ety_pin->set_text("");});
    v_btn_nip_enter->signal_clicked().connect(sigc::mem_fun(*this, &VBaseNip::on_btn_nip_enter));
}

VBaseNip::~VBaseNip()
{
}
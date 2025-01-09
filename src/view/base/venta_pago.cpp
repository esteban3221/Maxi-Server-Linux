#include "view/base/venta_pago.hpp"


BVentaPago::BVentaPago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                                Gtk::Box(cobject)
{
    v_lbl_cambio = m_builder->get_widget<Gtk::Label>("lbl_cambio");
    v_lbl_faltante = m_builder->get_widget<Gtk::Label>("lbl_faltante");
    v_lbl_mensaje_fin = m_builder->get_widget<Gtk::Label>("lbl_mensaje_fin");
    v_lbl_monto_total = m_builder->get_widget<Gtk::Label>("lbl_monto_total");
    v_lbl_recibido = m_builder->get_widget<Gtk::Label>("lbl_recibido");
    v_lbl_timeout = m_builder->get_widget<Gtk::Label>("lbl_timeout");
    v_lbl_titulo = m_builder->get_widget<Gtk::Label>("lbl_titulo");

    v_btn_timeout_cancel = m_builder->get_widget<Gtk::Button>("btn_timeout_cancel");
    v_btn_timeout_retry = m_builder->get_widget<Gtk::Button>("btn_timeout_retry");

    v_btn_timeout_cancel->signal_clicked().connect(sigc::mem_fun(*this, &BVentaPago::on_btn_cancel_click));
    v_btn_timeout_retry->signal_clicked().connect(sigc::mem_fun(*this, &BVentaPago::on_btn_retry_click));
}

BVentaPago::~BVentaPago()
{
}
#pragma once
#include <gtkmm.h>

class BVentaPago : public Gtk::Box
{
protected:
    Gtk::Label *v_lbl_monto_total, *v_lbl_recibido, *v_lbl_cambio, *v_lbl_faltante, *v_lbl_mensaje_fin, *v_lbl_titulo, *v_lbl_timeout;
    Gtk::Button *v_btn_timeout_cancel, *v_btn_timeout_retry;
    Gtk::ListBoxRow *v_BXRW4;

    virtual void on_btn_retry_click() = 0;
    virtual void on_btn_cancel_click() = 0;
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

public:
    BVentaPago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) ;
    virtual ~BVentaPago();
};

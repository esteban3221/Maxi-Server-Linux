#pragma once
#include <gtkmm.h>


class VMetodoPago : public Gtk::Box
{
protected:
    Gtk::Button* v_btn_cash;
    Gtk::Button* v_btn_card;
    Gtk::Button* v_btn_deferred;

private:
    Glib::RefPtr<Gtk::Builder> m_builder;

public:
    VMetodoPago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VMetodoPago();
};

namespace View
{
   extern const char *ui_metodo_pago;
}
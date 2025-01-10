#include "pago_manual.hpp"

PagoManual::PagoManual(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : BVentaPago(cobject,refBuilder)
{
    v_lbl_titulo->set_label("Pago Manual");
}

PagoManual::~PagoManual()
{
}

void PagoManual::on_btn_retry_click()
{
}

void PagoManual::on_btn_cancel_click()
{
}
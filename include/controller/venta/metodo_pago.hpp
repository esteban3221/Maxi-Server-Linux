#pragma once
#include "view/metodo_pago.hpp"
#include "model/log.hpp"

class MetodoPago : public VMetodoPago
{
private:
    enum class Metodo { EFECTIVO, TARJETA, MIXTO } metodo_seleccionado;
    Glib::RefPtr<MLog> m_log;
    Log log;

    void btn_efectivo_on_click();
    void btn_tarjeta_on_click();
    void btn_diferido_on_click();
    void reset_log();

    crow::response procesa_pago(const crow::request &req);

public:
    MetodoPago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~MetodoPago();
};
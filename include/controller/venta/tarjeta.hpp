#pragma once
#include "global.hpp"
#include "model/terminales.hpp" 
#include "model/log.hpp" 
#include "model/pago_terminal.hpp"
#include "model/detalle_movimiento.hpp"

class Tarjeta
{
private:
    const std::string MP_BASE_URL = "https://api.mercadopago.com/v1/orders";

    Glib::RefPtr<MLog> t_log;
    Log log;
    OPagoTarjeta pago_terminal;
    Glib::RefPtr<MPagoTarjeta> pago_actual;


public:
    Tarjeta();
    ~Tarjeta();

    // Crea una orden de pago
    cpr::Response create_order();
    cpr::Response cancel_order(const std::string &order_id);
    cpr::Response refund_order(const std::string &order_id, double amount = 0.0); 
    cpr::Response get_order_status(const std::string &order_id);

    void iniciar(Glib::RefPtr<MLog> t_log);
};
#pragma once
#include "global.hpp"
#include "model/terminales.hpp" // Potencialmente para integration_id
#include "model/log.hpp"           // Para registrar transacciones
#include "model/pago_terminal.hpp" // Para guardar detalles del pago

class Tarjeta
{
private:
    // La URL base para Point requiere el integration_id en la ruta
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
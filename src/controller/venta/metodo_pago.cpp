#include "controller/venta/metodo_pago.hpp"

MetodoPago::MetodoPago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VMetodoPago(cobject, refBuilder)
{
}

MetodoPago::~MetodoPago()
{
}
/*
    TODO:

    - Las instancias recibiran un t_log para actualizar el estatus de la venta, y se conectaran a las señales del hub para recibir eventos relacionados con el proceso de pago.
    Inicio (procesa_pago):
    
    Se crea el MLog en la base de datos.
    Se consulta default_payment. Aquí el sistema decide el "camino feliz".

Camino 0 (Efectivo):

    Va directo a la vista de Venta. El aceptador de billetes se enciende y el flujo termina cuando se completa el monto o se cancela.

Camino 1 (Tarjeta):

    Va a la vista de Tarjeta. Si la transacción es exitosa por el total, se cierra la venta.

Camino 2 (Mixto - lógica personalizada):

    Se queda en la vista MetodoPago.
    Bucle de Tarjeta: El usuario ingresa un monto en el Dialpad. Se procesa con tarjeta.
    Actualización del Log: Al terminar la tarjeta, se suma al m_ingreso del log.
    ¿Falta dinero?: * Si ya se cubrió el total, finaliza.
        Si falta, el usuario decide: ¿Otro abono con tarjeta o el resto con efectivo?

    Cierre con Efectivo: Una vez que se selecciona efectivo, se bloquea el Dialpad y se salta a la vista de Venta para cobrar el remanente físico.

*/  



crow::response MetodoPago::procesa_pago(const crow::request &req)
{   
    auto param = crow::json::load(req.body);
    auto is_view_ingreso = param.has("is_view_ingreso") && param["is_view_ingreso"].b();

    m_log = MLog::create
    (
        0,
        Global::User::id,
        is_view_ingreso ? "Ingreso" : "Venta",
        param["concepto"].operator std::string(),
        0,
        0,
        param["value"].i(),
        "Creacion de evento",
        Glib::DateTime::create_now_local()
    );

    m_log->m_id = log.insert_log(m_log);

    return {};
}
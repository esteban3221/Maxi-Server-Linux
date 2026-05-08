#include "model/pago_terminal.hpp"

OPagoTarjeta::OPagoTarjeta()
{

}
OPagoTarjeta::~OPagoTarjeta()
{

}

void OPagoTarjeta::insertar(const Glib::RefPtr<MPagoTarjeta>& pago)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("INSERT INTO pago_tarjeta (id, id_terminal, id_order, mp_order_id, mp_payment_id, id_log, monto, tipo, estado, estado_detalle, mp_reference_id, mp_transaction_id, last_four_digits, issuer_name, fecha_creado, fecha_aprobacion, fecha_ult_actualizacion, descripcion, ultimo_error, mp_json_response) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                                pago->m_id,
                                pago->m_id_terminal.c_str(),
                                pago->m_id_order.c_str(),
                                pago->m_mp_order_id.c_str(),
                                pago->m_mp_payment_id.c_str(),
                                pago->m_id_log,
                                pago->m_monto,
                                pago->m_tipo.c_str(),
                                pago->m_estado.c_str(),
                                pago->m_estado_detalle.c_str(),
                                pago->m_mp_reference_id.c_str(),
                                pago->m_mp_transaction_id.c_str(),
                                pago->m_last_four_digits.c_str(),
                                pago->m_issuer_name.c_str(),
                                pago->m_fecha_creado.format_iso8601().c_str(),
                                pago->m_fecha_aprobacion.format_iso8601().c_str(),
                                pago->m_fecha_ult_actualizacion.format_iso8601().c_str(),
                                pago->m_descripcion.c_str(),
                                pago->m_ultimo_error.c_str(),
                                pago->m_mp_json_response.c_str());
}

void OPagoTarjeta::actualizar_estado(const Glib::ustring& id_order, const Glib::ustring& nuevo_estado, const Glib::ustring& detalle)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("UPDATE pago_tarjeta SET estado = ?, estado_detalle = ?, fecha_ult_actualizacion = ? WHERE id_order = ?",
                                nuevo_estado.c_str(),
                                detalle.c_str(),
                                Glib::DateTime::create_now_local().format_iso8601().c_str(),
                                id_order.c_str());
}

void OPagoTarjeta::vincular_pago_mp(const Glib::ustring& id_order, const Glib::ustring& mp_payment_id)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("UPDATE pago_tarjeta SET mp_payment_id = ? WHERE id_order = ?",
                                mp_payment_id.c_str(),
                                id_order.c_str());
}

Glib::RefPtr<MPagoTarjeta> OPagoTarjeta::get_by_id_order(const Glib::ustring& id_order)
{
    auto &database = Database::getInstance();
    auto contenedor_data = database.sqlite3->command("SELECT * FROM pago_tarjeta WHERE id_order = ?", id_order);

    if (contenedor_data->at("id").empty())
        return nullptr;

    return MPagoTarjeta::create(
        std::stoul(contenedor_data->at("id")[0]),
        contenedor_data->at("id_terminal")[0],
        contenedor_data->at("id_order")[0],
        contenedor_data->at("mp_order_id")[0],
        contenedor_data->at("mp_payment_id")[0],
        std::stoul(contenedor_data->at("id_log")[0]),
        std::stod(contenedor_data->at("monto")[0]),
        contenedor_data->at("tipo")[0],
        contenedor_data->at("estado")[0],
        contenedor_data->at("estado_detalle")[0],
        contenedor_data->at("mp_reference_id")[0],
        contenedor_data->at("mp_transaction_id")[0],
        contenedor_data->at("last_four_digits")[0],
        contenedor_data->at("issuer_name")[0],
        Glib::DateTime::create_from_iso8601(contenedor_data->at("fecha_creado")[0]),
        Glib::DateTime::create_from_iso8601(contenedor_data->at("fecha_aprobacion")[0]),
        Glib::DateTime::create_from_iso8601(contenedor_data->at("fecha_ult_actualizacion")[0]),
        contenedor_data->at("descripcion")[0],
        contenedor_data->at("ultimo_error")[0],
        contenedor_data->at("mp_json_response")[0]);
}

Glib::RefPtr<MPagoTarjeta> OPagoTarjeta::get_by_mp_order_id(const Glib::ustring& mp_order_id)
{
    auto &database = Database::getInstance();
    auto contenedor_data = database.sqlite3->command("SELECT * FROM pago_tarjeta WHERE mp_order_id = ?", mp_order_id);

    if (contenedor_data->at("id").empty())
        return nullptr;

    return MPagoTarjeta::create(
        std::stoul(contenedor_data->at("id")[0]),
        contenedor_data->at("id_terminal")[0],
        contenedor_data->at("id_order")[0],
        contenedor_data->at("mp_order_id")[0],
        contenedor_data->at("mp_payment_id")[0],
        std::stoul(contenedor_data->at("id_log")[0]),
        std::stod(contenedor_data->at("monto")[0]),
        contenedor_data->at("tipo")[0],
        contenedor_data->at("estado")[0],
        contenedor_data->at("estado_detalle")[0],
        contenedor_data->at("mp_reference_id")[0],
        contenedor_data->at("mp_transaction_id")[0],
        contenedor_data->at("last_four_digits")[0],
        contenedor_data->at("issuer_name")[0],
        Glib::DateTime::create_from_iso8601(contenedor_data->at("fecha_creado")[0]),
        Glib::DateTime::create_from_iso8601(contenedor_data->at("fecha_aprobacion")[0]),
        Glib::DateTime::create_from_iso8601(contenedor_data->at("fecha_ult_actualizacion")[0]),
        contenedor_data->at("descripcion")[0],
        contenedor_data->at("ultimo_error")[0],
        contenedor_data->at("mp_json_response")[0]);
}
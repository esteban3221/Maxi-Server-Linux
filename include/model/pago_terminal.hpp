#pragma once
#include <iostream>
#include <glibmm.h>
#include <giomm.h>

#include "coneccion.hpp"

class MPagoTarjeta : public Glib::Object
{
public:
    // Campos de la base de datos
    size_t m_id;
    Glib::ustring m_id_terminal;
    Glib::ustring m_id_order;
    Glib::ustring m_mp_order_id;
    Glib::ustring m_mp_payment_id; // Opcional
    size_t m_id_log;
    double m_monto;
    Glib::ustring m_tipo;
    Glib::ustring m_estado;
    Glib::ustring m_estado_detalle;
    Glib::ustring m_mp_reference_id;   // Opcional
    Glib::ustring m_mp_transaction_id; // Opcional
    Glib::ustring m_last_four_digits;  // Opcional
    Glib::ustring m_issuer_name;       // Opcional
    Glib::DateTime m_fecha_creado;
    Glib::DateTime m_fecha_aprobacion;       // Opcional
    Glib::DateTime m_fecha_ult_actualizacion;
    Glib::ustring m_descripcion;
    Glib::ustring m_ultimo_error;    // Opcional
    Glib::ustring m_mp_json_response; // Opcional (JSON para debug)

    static Glib::RefPtr<MPagoTarjeta> create(size_t id,
                                              const Glib::ustring& id_terminal,
                                              const Glib::ustring& id_order,
                                              const Glib::ustring& mp_order_id,
                                              const Glib::ustring& mp_payment_id,
                                              size_t id_log,
                                              double monto,
                                              const Glib::ustring& tipo,
                                              const Glib::ustring& estado,
                                              const Glib::ustring& estado_detalle,
                                              const Glib::ustring& mp_reference_id,
                                              const Glib::ustring& mp_transaction_id,
                                              const Glib::ustring& last_four_digits,
                                              const Glib::ustring& issuer_name,
                                              const Glib::DateTime& fecha_creado,
                                              const Glib::DateTime& fecha_aprobacion,
                                              const Glib::DateTime& fecha_ult_actualizacion,
                                              const Glib::ustring& descripcion,
                                              const Glib::ustring& ultimo_error,
                                              const Glib::ustring& mp_json_response) {
        return Glib::make_refptr_for_instance<MPagoTarjeta>(new MPagoTarjeta(id, 
                                                                             id_terminal, 
                                                                             id_order, 
                                                                             mp_order_id, 
                                                                             mp_payment_id, 
                                                                             id_log, 
                                                                             monto, 
                                                                             tipo, 
                                                                             estado, 
                                                                             estado_detalle, 
                                                                             mp_reference_id, 
                                                                             mp_transaction_id, 
                                                                             last_four_digits, 
                                                                             issuer_name, 
                                                                             fecha_creado, 
                                                                             fecha_aprobacion, 
                                                                             fecha_ult_actualizacion, 
                                                                             descripcion, 
                                                                             ultimo_error, 
                                                                             mp_json_response));
    }

protected:
    MPagoTarjeta(size_t id,
                const Glib::ustring& id_terminal,
                const Glib::ustring& id_order,
                const Glib::ustring& mp_order_id,
                const Glib::ustring& mp_payment_id,
                size_t id_log,
                double monto,
                const Glib::ustring& tipo,
                const Glib::ustring& estado,
                const Glib::ustring& estado_detalle,
                const Glib::ustring& mp_reference_id,
                const Glib::ustring& mp_transaction_id,
                const Glib::ustring& last_four_digits,
                const Glib::ustring& issuer_name,
                const Glib::DateTime& fecha_creado,
                const Glib::DateTime& fecha_aprobacion,
                const Glib::DateTime& fecha_ult_actualizacion,
                const Glib::ustring& descripcion,
                const Glib::ustring& ultimo_error,
                const Glib::ustring& mp_json_response) 
        : m_id(id),
          m_id_terminal(id_terminal),
          m_id_order(id_order),
          m_mp_order_id(mp_order_id),
          m_mp_payment_id(mp_payment_id),
          m_id_log(id_log),
          m_monto(monto),
          m_tipo(tipo),
          m_estado(estado),
          m_estado_detalle(estado_detalle),
          m_mp_reference_id(mp_reference_id),
          m_mp_transaction_id(mp_transaction_id),
          m_last_four_digits(last_four_digits),
          m_issuer_name(issuer_name),
          m_fecha_creado(fecha_creado),
          m_fecha_aprobacion(fecha_aprobacion),
          m_fecha_ult_actualizacion(fecha_ult_actualizacion),
          m_descripcion(descripcion),
          m_ultimo_error(ultimo_error),
          m_mp_json_response(mp_json_response) 
    {
    }
};

class OPagoTarjeta
{
public:
    OPagoTarjeta();
    ~OPagoTarjeta();

    // Métodos de persistencia
    void insertar(const Glib::RefPtr<MPagoTarjeta>& pago);
    void actualizar_estado(const Glib::ustring& id_order, const Glib::ustring& nuevo_estado, const Glib::ustring& detalle);
    void vincular_pago_mp(const Glib::ustring& id_order, const Glib::ustring& mp_payment_id);
    
    Glib::RefPtr<MPagoTarjeta> get_by_id_order(const Glib::ustring& id_order);
    Glib::RefPtr<MPagoTarjeta> get_by_mp_order_id(const Glib::ustring& mp_order_id);
};
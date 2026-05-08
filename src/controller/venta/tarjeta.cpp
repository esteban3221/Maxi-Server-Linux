#include "controller/venta/tarjeta.hpp"

Tarjeta::Tarjeta()
{
}

Tarjeta::~Tarjeta()
{
}

cpr::Response Tarjeta::create_order()
{
    auto terminal = OTerminales().obtener_predeterminado();
    auto idempotency_base = "ext_ref_" + std::to_string(t_log->m_id) + "_" + std::to_string(t_log->m_total);

    crow::json::wvalue json_params;
    json_params["type"] = "point";
    json_params["external_reference"] = "ext_ref_" + std::to_string(t_log->m_id);
    json_params["expiration_time"] = "PT10M";
    json_params["description"] = std::to_string(t_log->m_id) + "_" + std::to_string(t_log->m_total);

    crow::json::wvalue payment;
    payment["amount"] = std::to_string(t_log->m_total);
    json_params["transactions"]["payments"] = crow::json::wvalue::list({payment});
    json_params["config"]["point"]["terminal_id"] = terminal ? terminal->m_id : "";
    json_params["config"]["point"]["print_on_terminal"] = "no_ticket";
    json_params["config"]["payment_method"]["default_type"] = "credit_card";

    CROW_LOG_INFO << "Mercado Pago: Creando orden...\n" << json_params.dump();
    return cpr::Post(cpr::Url{MP_BASE_URL},
                     cpr::Header{{"Authorization", "Bearer " + terminal->m_access_token},
                                 {"X-Idempotency-Key", idempotency_base},
                                 {"Content-Type", "application/json"}},
                     cpr::Body{json_params.dump()});
}

cpr::Response Tarjeta::cancel_order(const std::string &order_id)
{
    CROW_LOG_INFO << "Mercado Pago: Cancelando orden " << order_id << "...";
    return cpr::Post(cpr::Url{MP_BASE_URL, order_id, "/cancel"},
                     cpr::Header{{"Authorization", "Bearer " + OTerminales().obtener_predeterminado()->m_access_token},
                                 {"Content-Type", "application/json"}});
}

cpr::Response Tarjeta::refund_order(const std::string &order_id, double amount)
{
    CROW_LOG_INFO << "Mercado Pago: Reembolsando orden " << order_id << " por monto " << amount << "...";

    return {};
}

cpr::Response Tarjeta::get_order_status(const std::string &order_id)
{
    auto terminal = OTerminales().obtener_predeterminado();
    bool termino = false;
    cpr::Response response;

    // std::thread([this, terminal, order_id, &termino, &response]()
    // {
        while (!termino)
        {
            response = cpr::Get(cpr::Url{MP_BASE_URL, "/", order_id},
                                cpr::Header{{"Authorization", "Bearer " + terminal->m_access_token},
                                            {"Content-Type", "application/json"}});
            if (response.status_code == 200)
            {
                auto json_response = crow::json::load(response.text);
                pago_actual->m_estado = json_response["status"].s();
                pago_actual->m_estado_detalle = json_response["status_detail"].s();
                pago_actual->m_fecha_ult_actualizacion = Glib::DateTime::create_from_iso8601(std::string(json_response["last_updated_date"].s()));
                pago_terminal.actualizar_estado(order_id, pago_actual->m_estado, pago_actual->m_estado_detalle);

                if (pago_actual->m_estado == "processed" ||
                    pago_actual->m_estado == "expired" ||
                    pago_actual->m_estado == "canceled" ||
                    pago_actual->m_estado == "failed" ||
                    pago_actual->m_estado == "refunded" ||
                    pago_actual->m_estado == "action_required") 
                {
                    termino = true;
                }
                
            }
            else
            {
                CROW_LOG_ERROR << "Mercado Pago: Error al obtener el estado de la orden " << order_id 
                               << "\nCódigo de estado: " << response.status_code
                               << "\nRespuesta: " << response.text;
                termino = true;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        } 
    // }).detach();
    return response;
}

void Tarjeta::iniciar(Glib::RefPtr<MLog> t_log)
{
    //Global::Widget::v_main_stack
    this->t_log = t_log;
    if(t_log->m_total <= 5)
    {
        Global::System::showNotify("Tarjeta","El monto mínimo para procesar un pago con tarjeta es de $5. Por favor, ajuste el monto e intente nuevamente.", "dialog-warning");
        return;
    }

    auto reponse = create_order();

    if (reponse.status_code != 201)
    {
        CROW_LOG_ERROR << "Mercado Pago: Error al crear la orden para el log " << t_log->m_id 
                    << "\nCódigo de estado: " << reponse.status_code
                    << "\nRespuesta: " << reponse.text;

        Global::System::showNotify("Tarjeta","Error en el pago con tarjeta, No se pudo crear la orden de pago. Por favor, intente nuevamente o contacte al soporte.", "dialog-error");
        return;
    }

    auto json_response = crow::json::load(reponse.text);
    pago_actual = MPagoTarjeta::create
    (
        0,                                                        // id
        std::string(json_response["config"]["point"]["terminal_id"].s()), // id_terminal
        std::string(json_response["id"].s()),                     // id_order (ID interno de MP)
        std::string(json_response["transactions"]["payments"][0]["id"].s()), // mp_order_id
        "",                                                       // mp_payment_id
        t_log->m_id,                                              // id_log
        t_log->m_total,                                           // monto
        "card",                                                   // tipo
        std::string(json_response["status"].s()),                 // estado
        std::string(json_response["status_detail"].s()),          // estado_detalle
        "",                                                       // mp_reference_id (NUEVO)
        "ext_ref_" + std::to_string(t_log->m_id) + "_" + std::to_string(t_log->m_total),// mp_transaction_id (NUEVO)
        "",                                                       // last_four_digits
        "",                                                       // issuer_name
        Glib::DateTime::create_from_iso8601(std::string(json_response["created_date"].s())),      // fecha_creado
        Glib::DateTime::create_now_local(),                       // fecha_aprobacion (FALTABA - puedes poner la actual o nula)
        Glib::DateTime::create_from_iso8601(std::string(json_response["last_updated_date"].s())), // fecha_ult_actualizacion
        t_log->m_descripcion,                                     // descripcion
        "",                                                       // ultimo_error
        reponse.text                                             // mp_json_response (CORREGIDO typo y coma)
    );

    pago_terminal.insertar(pago_actual);
    auto response_status = get_order_status(pago_actual->m_id_order);

    if (pago_actual->m_estado == "processed")
    {
        t_log->m_ingreso += t_log->m_total;
        t_log->m_estatus = pago_actual->m_estado;
        log.update_log(t_log);
        Global::System::showNotify("Tarjeta","Pago con tarjeta aprobado exitosamente.", "dialog-information");
    }
    else
    {
        t_log->m_estatus += '\n' + pago_actual->m_estado + " - " + pago_actual->m_estado_detalle;
        Global::System::showNotify("Tarjeta","El pago con tarjeta no se pudo procesar. Por favor, intente nuevamente o contacte al soporte.", "dialog-error");
    }
}
#include "core/context/validador.hpp"
#include "core/states/svalidador.hpp"

ValidadorUnit::ValidadorUnit(/* args */) : handler_state(std::make_unique<EstadoIdle>()),
                                           device_id("Desconocido"),
                                           ingreso_credito(0),
                                           salida_credito(0),
                                           poll_milli(200),
                                           poll(false)
{
    handler_state->on_entry(*this);
}

ValidadorUnit::~ValidadorUnit()
{
}

// context/validador.cpp (implementación)
void ValidadorUnit::transiciona_estado(std::unique_ptr<IValidador> nuevo_estado)
{
    if (!nuevo_estado)
        return;

    if (handler_state)
    {
        handler_state->on_exit(*this);
    }

    auto estado_anterior = handler_state ? handler_state->get_nombre_estado() : "Ninguno";
    handler_state = std::move(nuevo_estado);

    handler_state->on_entry(*this);
    signal_state_changed.emit(handler_state->get_nombre_estado());

    CROW_LOG_INFO << "Transición: " << estado_anterior << " -> " << handler_state->get_nombre_estado() << '\n';
}

void ValidadorUnit::imprime_debug(const cpr::Response &r) const
{
    CROW_LOG_DEBUG << "Validador:" << device_id << "\n"
                   << "URL: " << r.url.str() << '\n'
                   << "Tiempo: " << r.elapsed << '\n'
                   << "Codigo: " << r.status_code << '\n'
                   << "Respuesta: " << r.text << '\n';
}

const cpr::Response ValidadorUnit::command_get(const std::string &command, bool debug) const
{
    auto response = cpr::Get(cpr::Url{BASE_URL, "/", command, "?deviceID=", device_id},
                             cpr::Header{{"Content-Type", "application/json"},
                                         {"Authorization", "Bearer " + token}});

    if (debug)
        imprime_debug(response);

    return response;
}
const cpr::Response ValidadorUnit::command_post(const std::string &command, const std::string &json, bool debug) const
{
    auto response = cpr::Post(cpr::Url{BASE_URL, "/", command, "?deviceID=", device_id},
                              cpr::Header{{"Content-Type", "application/json"},
                                          {"Authorization", "Bearer " + token}},
                              cpr::Body{json});

    if (debug)
        imprime_debug(response);

    return response;
}

/*
{
"ComPort": "{{ComPort}}",
"SspAddress": {{SspAddress}},
"LogFilePath": "C:\\Temp\\",
"SetInhibits": [
{ "Denomination": "500 {{Currency}}", "Inhibit": false },
{ "Denomination": "1000 {{Currency}}", "Inhibit": false },
{ "Denomination": "2000 {{Currency}}", "Inhibit": false },
{ "Denomination": "5000 {{Currency}}", "Inhibit": false },
{ "Denomination": "10000 {{Currency}}", "Inhibit": false }
],
"SetRoutes": [
{ "Denomination": "500 {{Currency}}", "Route": 1 },
{ "Denomination": "1000 {{Currency}}", "Route": 1 },
{ "Denomination": "2000 {{Currency}}", "Route": 1 },
{ "Denomination": "5000 {{Currency}}", "Route": 0 }
],
"EnableAcceptor": true,
"EnableAutoAcceptEscrow": true,
"En
*/

crow::json::rvalue  ValidadorUnit::inicia_conecta(const Conf &conf, const crow::json::rvalue &set_routes)
{
    auto json = crow::json::wvalue{
        {"ComPort", conf.puerto},
        {"SspAddress", conf.ssp},
        {"LogFilePath", conf.log_ruta},
        {"SetInhibits", crow::json::wvalue::list()}, // Hasta el momento no hay ninguna necesidad de inhibir denominaciones, pero se puede agregar fácilmente aquí
        {"SetRoutes", set_routes},
        {"EnableAcceptor", conf.habilita_recolector},
        {"EnableAutoAcceptEscrow", conf.auto_acepta_credito},
        {"EnableAutoRouteCredit", conf.habilita_salida_credito}
    };

    auto response = command_post("OpenConnection", json.dump(), true);
    auto json_response = crow::json::load(response.text);
    if (response.status_code == 200)
    {
        device_id = json_response["deviceID"].s();
        CROW_LOG_INFO << "Conectado al validador con ID: " << device_id << '\n';
        return json_response["allLevels"];
    }
    else
        CROW_LOG_ERROR << "Error al conectar al validador: " << response.text << '\n';
        // transicionar a un estado de Error automáticamente
        // v.transiciona_estado(std::make_unique<EstadoError>());
    
    return {};
}

void ValidadorUnit::detiene_desconecta()
{
    if(poll) {
        poll = false; // Detiene el polling
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_milli + 100)); // Espera a que el hilo de polling termine
    }

    auto response = command_post("DisconnectDevice", "", true);
    if (response.status_code == 200)
    {
        CROW_LOG_INFO << response.text << '\n';
        device_id = "Desconocido";
    }
    else
    {
        CROW_LOG_ERROR << "Error al desconectar del validador: " << response.text << '\n';
        // transicionar a un estado de Error automáticamente
        // v.transiciona_estado(std::make_unique<EstadoError>());
    }
}
const std::string ValidadorUnit::get_nombre_estado()
{
    return handler_state->get_nombre_estado();
}
void ValidadorUnit::iniciar_polling() {
    poll = true; 
    std::thread([this]() {
        while (poll) {
            auto resp = command_get("GetDeviceStatus/v2");
            if (resp.status_code == 200) {
                auto json = crow::json::load(resp.text);
                for (const auto &item : json["pollBuffer"]) {
                    std::string event_name;
                    if (item.has("stateAsString")) event_name = item["stateAsString"].s();
                    else if (item.has("eventTypeAsString")) event_name = item["eventTypeAsString"].s();

                    // IMPORTANTE: Delegamos la decisión al estado actual
                    handler_state->on_handle_event(*this, event_name, item);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(poll_milli));
        }
    }).detach();
}
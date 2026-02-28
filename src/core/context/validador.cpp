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

    CROW_LOG_INFO << "Transición: " << estado_anterior << " -> " << handler_state->get_nombre_estado();
}

void ValidadorUnit::imprime_debug(const std::string &command, const cpr::Response &r) const
{
    std::puts("");
    CROW_LOG_DEBUG << "Validador:" << device_id << '\n'
                   << "\t\t\t\t Comando: " << command << '\n'
                   << "\t\t\t\t Tiempo: " << r.elapsed << '\n'
                   << "\t\t\t\t Codigo: " << r.status_code << '\n'
                   << "\t\t\t\t Respuesta: " << r.text << '\n';
}

const cpr::Response ValidadorUnit::command_get(const std::string &command, bool debug) const
{
    auto response = cpr::Get(cpr::Url{BASE_URL, command, "?deviceID=", device_id},
                             cpr::Header{{"Content-Type", "application/json"},
                                         {"Authorization", "Bearer " + token}});

    if (debug)
        imprime_debug(command, response);

    return response;
}
const cpr::Response ValidadorUnit::command_post(const std::string &command, const std::string &json, bool debug) const
{
    auto response = cpr::Post(cpr::Url{BASE_URL, command, "?deviceID=", device_id},
                              cpr::Header{{"Content-Type", "application/json"},
                                          {"Authorization", "Bearer " + token}},
                              cpr::Body{json} /*,
                               cpr::Timeout{2000}*/
    );

    if (debug)
        imprime_debug(command, response);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
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

const crow::json::rvalue ValidadorUnit::inicia_conecta(const crow::json::rvalue &set_routes)
{
    auto json = crow::json::wvalue{
        {"ComPort", conf.puerto},
        {"SspAddress", conf.ssp},
        {"LogFilePath", conf.log_ruta},
        {"SetInhibits", crow::json::wvalue::list()}, // Hasta el momento no hay ninguna necesidad de inhibir denominaciones, pero se puede agregar fácilmente aquí
        {"SetRoutes", set_routes},
        {"EnableAcceptor", conf.habilita_recolector},
        {"EnableAutoAcceptEscrow", conf.auto_acepta_credito},
        {"EnablePayout", conf.habilita_salida_credito}};

    auto response = command_post("OpenConnection", json.dump() /*, true*/);
    auto json_response = crow::json::load(response.text);
    if (response.status_code == 200)
    {
        device_id = json_response["deviceID"].s();
        device_model = json_response["deviceModel"].s();

        CROW_LOG_INFO << "Conectado al validador con ID: " << device_id;
        // transiciona_estado(std::make_unique<EstadoActivo>());
        return json_response["allLevels"];
    }
    else
        transiciona_estado(std::make_unique<EstadoError>("Error al conectar al validador: " + device_id));

    return {};
}

void ValidadorUnit::detiene_desconecta()
{
    // transiciona_estado(std::make_unique<EstadoDeteniendo>());
    if (poll.load())
    {
        poll = false;                                                             // Detiene el polling
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_milli + 100)); // Espera a que el hilo de polling termine
    }

    auto response = command_post("DisconnectDevice");
    if (response.status_code == 200)
    {
        CROW_LOG_INFO << response.text;
        device_id = "Desconocido";
        // transiciona_estado(std::make_unique<EstadoIdle>());
    }
    else
        transiciona_estado(std::make_unique<EstadoError>("Error al desconectar del validador: " + response.text));
}

const std::string ValidadorUnit::get_nombre_estado()
{
    return handler_state->get_nombre_estado();
}

void ValidadorUnit::iniciar_polling()
{
    poll = true;
    std::thread([this]()
    {
        while (poll) {
            auto resp = command_get("GetDeviceStatus/v2");
            if (resp.status_code == cpr::status::HTTP_OK) {
                auto json = crow::json::load(resp.text);
                for (const auto &item : json["pollBuffer"]) {
                    std::string event_name;
                    if (item.has("stateAsString")) 
                        event_name = item["stateAsString"].s();
                    else if (item.has("eventTypeAsString")) 
                        event_name = item["eventTypeAsString"].s();
                    if (event_name == "ESCROW" || event_name == "STACKED" || event_name == "VALUE_ADDED") {
                        CROW_LOG_INFO << "Crédito detectado en Estado Activo: " << event_name;
                        signal_event_received.emit(event_name, item);
                    } 
                    else if (event_name == "FRAUD_ATTEMPT") {
                        signal_error.emit("Posible intento de fraude detectado");
                    }
                    else if (event_name == "JAMMED") {
                        signal_error.emit("Dispositivo atascado");
                        // transicionar a un estado de Error automáticamente
                        transiciona_estado(std::make_unique<EstadoError>("Atasco detectado"));
                    }
                    else if (event_name == "INCOMPLETE_PAYOUT" || event_name == "ERROR_DURING_PAYOUT") {
                        signal_error.emit("Pago incompleto detectado");
                        // transicionar a un estado de Error automáticamente
                        transiciona_estado(std::make_unique<EstadoError>("Error genérico detectado: " ));
                    }
                    else if (event_name == "ERROR")
                    {
                        signal_error.emit("Error genérico detectado" );
                        // transicionar a un estado de Error automáticamente
                        transiciona_estado(std::make_unique<EstadoError>("Error genérico detectado: " ));
                    }
                    else if (event_name == "TIME_OUT")
                    {
                        signal_error.emit("Tiempo limite alcanzado" );
                        // transicionar a un estado de Error automáticamente
                        transiciona_estado(std::make_unique<EstadoError>("Error genérico detectado: " ));
                    }
                    else if (event_name == "IN_PROGRESS")
                    {
                        continue;
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(poll_milli));
        } 
        CROW_LOG_INFO << "Terminando Polleo para el dispositivo" << device_id; 
    }).detach();
}

uint ValidadorUnit::iniciar_pago(size_t monto)
{   
    uint cambio = monto;
    auto response = command_get("GetAllLevels");
    auto json = crow::json::load(response.text);
    std::map<int, int> levels;

    for (auto &&i : json) levels[i["value"].i() / 100] = i["storedInPayout"].i();
    auto json_pago = obten_cambio(cambio, levels, false);

    if (cambio > 0)
        signal_error.emit("No se cuenta con suficiente efectivo para dar cambio en el dispositivo: " + device_id);
    else
        iniciar_pago(json_pago.dump());

    return cambio;
}

void ValidadorUnit::iniciar_pago(const std::string &denom)
{
    if (denom.size() == 0) return;
    //if(denom.begin()->i() == 0 && denom.end()->i() == 0) return; hay que hacer algun mecanismo para detectar si esta vacio o llenos de ceros

    auto response = command_post("PayoutMultipleDenominations", denom);
    if (response.status_code == cpr::status::HTTP_OK)
        iniciar_polling();
}

crow::json::wvalue ValidadorUnit::obten_cambio(uint &cambio, std::map<int, int> &reciclador, bool is_cambio)
{
    std::vector<int> billsToReturn(reciclador.size(), 0); // Vector para almacenar la cantidad de billetes a devolver
    auto cambio_original = cambio; // Guardamos el valor original del cambio 
    int index = reciclador.size() - 1; // Índice para insertar en el vector
    for (auto it = reciclador.rbegin(); it != reciclador.rend(); ++it, --index)
    {
        int denominacion = it->first;         // Denominación del billete
        int &cantidadDisponible = it->second; // Cantidad disponible en el reciclado
        while ((is_cambio ? denominacion < cambio_original && cambio > 0 : cambio >= denominacion) && cantidadDisponible > 0)
        {
            cambio -= denominacion; // Reducir el cambio
            cantidadDisponible--;   // Usar un billete del reciclador
            billsToReturn[index]++; // Incrementar el contador de billetes devueltos
        }
    }
    // Convertimos el vector a JSON
    crow::json::wvalue response;
    response = billsToReturn;
    return response; // Retornamos el JSON con el array
}
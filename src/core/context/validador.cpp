#include "core/context/validador.hpp"
#include "core/states/svalidador.hpp"

ValidadorUnit::ValidadorUnit(/* args */) : device_id("Desconocido"),
                                           ingreso_credito(0),
                                           salida_credito(0),
                                           poll_milli(200),
                                           poll(false)
{
    //crow::logger::setLogLevel(crow::LogLevel::Debug);
}

ValidadorUnit::~ValidadorUnit()
{
    // detiene_desconecta();
}

std::string pretty_json(const std::string& json, const std::string& indent_prefix) {
    if (json.empty() || json[0] != '{' && json[0] != '[') return json;

    std::string out;
    out.reserve(json.length() * 2); // Evita realocaciones constantes
    int level = 0;
    bool in_quotes = false;

    for (size_t i = 0; i < json.length(); ++i) {
        char c = json[i];
        if (c == '"' && (i == 0 || json[i-1] != '\\')) in_quotes = !in_quotes;
        if (in_quotes) { out += c; continue; }

        if (c == '{' || c == '[') {
            out += c;
            out += "\n" + indent_prefix + std::string(++level * 4, ' ');
        } else if (c == '}' || c == ']') {
            out += "\n" + indent_prefix + std::string(--level * 4, ' ');
            out += c;
        } else if (c == ',') {
            out += ",\n" + indent_prefix + std::string(level * 4, ' ');
        } else if (c == ':') {
            out += ": ";
        } else if (!std::isspace(c)) {
            out += c;
        }
    }
    return out;
}

void ValidadorUnit::imprime_debug(const std::string &command, const cpr::Response &r, const std::string &body) const
{
    std::string prefix = "\t\t\t\t ";
    std::puts("\n==================== [Debug Interno] ====================");
    CROW_LOG_INFO << "Validador:" << device_id << '\n'
                   << prefix << "Comando: " << command << '\n'
                   << prefix << "Enviado: " << body << '\n'
                   << prefix << "Tiempo: " << r.elapsed << '\n'
                   << prefix << "Codigo: " << r.status_code << '\n'
                #ifdef MODO_DEBUG_JSON
                    << prefix << "Respuesta (pretty): " << pretty_json(r.text, prefix) << '\n';
                #else
                   << prefix << "Respuesta: " << r.text << '\n';
                #endif
}

const cpr::Response ValidadorUnit::command_get(const std::string &command, bool debug) const
{
    auto response = cpr::Get(cpr::Url{BASE_URL, command, "?deviceID=", device_id},
                             cpr::Header{{"Content-Type", "application/json"},
                                         {"Authorization", "Bearer " + token}});

    response.header["X-Device-Type"] = conf.ssp == 16 ? "COIN" : "BILL";

    if (debug)
        imprime_debug(command, response);

    return response;
}

const cpr::Response ValidadorUnit::command_post(const std::string &command, const std::string &json, bool debug) const
{
    auto response = cpr::Post(cpr::Url{BASE_URL, command, "?deviceID=", device_id},
                              cpr::Header{{"Content-Type", "application/json"},
                                          {"Authorization", "Bearer " + token}},
                              cpr::Body{json});

    response.header["X-Device-Type"] = conf.ssp == 16 ? "COIN" : "BILL";

    if (debug)
        imprime_debug(command, response, json);
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
        {"SetRoutes", set_routes.has("SetRoutes") ? set_routes["SetRoutes"] : set_routes},
        {"EnableAcceptor", conf.habilita_recolector},
        {"EnableAutoAcceptEscrow", conf.auto_acepta_credito},
        {"EnablePayout", conf.habilita_salida_credito}};

    auto response = command_post("OpenConnection", json.dump());
    auto json_response = crow::json::load(response.text);
    if (response.status_code == 200)
    {
        device_id = json_response["deviceID"].s();
        device_model = json_response["deviceModel"].s();

        CROW_LOG_INFO << "Conectado al validador con ID: " << device_id;
        ultimo_cash_level = command_get("GetAllLevels", true).text;
        return json_response["allLevels"];
    }
    else
        CROW_LOG_ERROR << "Error al conectar al validador: " << device_id;

    return {};
}

void ValidadorUnit::detiene_desconecta()
{
    if (poll.load())
    {
        poll = false;                                                             // Detiene el polling
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_milli)); // Espera a que el hilo de polling termine
    }
    auto response = command_post("DisconnectDevice");

    if (response.status_code == 200)
        CROW_LOG_INFO << device_id << " → " << response.text;
    else
        CROW_LOG_ERROR << "Error al desconectar del validador: " << response.text;
}

bool ValidadorUnit::esperar_pago_async() 
{
    bool detecto_dispensing = false;
    bool terminado = false;
    bool exito = false;
    bool nota_en_bezel = false; // Nueva bandera
    int reintentos_iniciales = 0;

    while (!terminado) {
        auto resp = command_get("GetDeviceStatus");
        if (resp.status_code == 200) 
        {
            auto json = crow::json::load(resp.text);
            std::string state = json.has("DeviceState") ? json["DeviceState"].s() : (json.has("deviceState") ? json["deviceState"].s() : "");

            if (state == "DISPENSING") detecto_dispensing = true;

            if (json.has("PollBuffer")) {
                for (const auto &item : json["PollBuffer"]) 
                {
                    std::string type = item.has("Type") ? std::string(item["Type"].s()) : "";
                    std::string stateStr = item.has("StateAsString") ? std::string(item["StateAsString"].s()) : "";
                    std::string eventStr = item.has("EventTypeAsString") ? std::string(item["EventTypeAsString"].s()) : "";

                    // Detectamos si el billete está en la boca (Bezel)
                    if (stateStr == "NOTE_HELD_IN_BEZEL" || eventStr == "NOTE_IN_BEZEL_HOLD") {
                        if (!nota_en_bezel) {
                            CROW_LOG_WARNING << "Billete en bezel. Esperando a que el usuario lo retire...";
                            nota_en_bezel = true;
                        }
                    }

                    // ÉXITO REAL: Solo cuando recibimos COMPLETED de la transacción
                    if (type == "DispenserTransactionEventResponse" && stateStr == "COMPLETED") {
                        exito = true;
                        terminado = true;
                    }
                    
                    // FALLOS
                    if (eventStr == "TIME_OUT" || eventStr == "JAMMED" || eventStr == "ERROR") {
                        exito = false;
                        terminado = true;
                    }
                }
            }

            // FAILSAFE: Si el estado es DISABLED o ENABLED pero ya no hay nada en el PollBuffer
            // y ya detectamos dispensing, significa que terminó aunque no vimos el evento.
            if (detecto_dispensing && (state == "DISABLED" || state == "IDLE" || state == "ENABLED")) {
                // Si ya no hay eventos de "HELD" en el buffer, podemos salir
                if (json["PollBuffer"].size() == 0 || (json["PollBuffer"].size() == 1 && json["PollBuffer"][0]["StateAsString"].s() == "DISABLED")) {
                    terminado = true;
                    // Si llegamos aquí sin 'exito = true', lo marcamos basado en que no hubo errores
                    if (!exito) exito = true; 
                }
            }

            if (!detecto_dispensing && ++reintentos_iniciales > 60) {
                terminado = true;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_milli));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(800)); 
    
    return exito;
}

void ValidadorUnit::iniciar_polling()
{
    poll = true;

    std::thread([this]()
                {
        while (poll) {
            auto resp = command_get("GetDeviceStatus");
            if (resp.status_code == cpr::status::HTTP_OK) 
            {
                auto json = crow::json::load(resp.text);
                for (const auto &item : json["pollBuffer"]) 
                {
                    std::string event_name;
                    if (item.has("stateAsString")) event_name = item["stateAsString"].s();
                    else if (item.has("eventTypeAsString")) event_name = item["eventTypeAsString"].s();

                    if (event_name == "ESCROW" || event_name == "STACKED" || event_name == "VALUE_ADDED") 
                    {
                        CROW_LOG_INFO << "Crédito detectado en Estado Activo: " << event_name << " - " << item["value"].i() / 100;
                        signal_event_received.emit(device_id, conf.ssp == 0 ? "BILL" : "COIN", event_name, item);
                    } 
                    else if (event_name == "FRAUD_ATTEMPT")
                        signal_error.emit(device_id, "Posible intento de fraude detectado");
                    else if (event_name == "JAMMED") 
                    {
                        signal_error.emit(device_id,"Dispositivo atascado");
                    }
                    else if (event_name == "ERROR")
                    {
                        signal_error.emit(device_id, "Error genérico detectado");
                        break;
                    }
                    else if (event_name == "CASHBOX_REMOVED")
                        signal_error.emit(device_id, event_name);
                    else if (event_name == "IN_PROGRESS")
                    {
                        continue;
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(poll_milli));
        } 
        CROW_LOG_INFO << "Terminando Polleo para el dispositivo " << device_id; })
        .detach();
}

uint ValidadorUnit::iniciar_pago(size_t monto, bool is_cambio, const std::string &actual_level)
{
    uint cambio = monto;
    std::map<int, int> levels;
    CROW_LOG_CRITICAL << "String actual_level: \n" << actual_level;
    auto json_levels = crow::json::load(actual_level);

    for (auto &&i : json_levels)
        levels[i["value"].i() / 100] = i["storedInPayout"].i();
    auto json_pago = obten_cambio(cambio, levels, is_cambio);

    iniciar_pago(json_pago.dump());

    return cambio;
}

void ValidadorUnit::iniciar_pago(const std::string &denom)
{
    try
    {
        if (denom.size() == 0 || denom == "[0,0,0,0,0,0]" || denom == "[0,0,0,0]")
            return;

        auto response = command_post("PayoutMultipleDenominations", denom, true);
        if (response.status_code == cpr::status::HTTP_OK)
        {
            auto future = std::async(std::launch::async, &ValidadorUnit::esperar_pago_async, this);
            CROW_LOG_INFO << "Bloqueando hilo hasta finalizar entrega física...";
            future.wait();
            CROW_LOG_INFO << "Entrega terminada. Continuando con el flujo.";
        }
        else if (auto json = crow::json::load(response.text); response.status_code == cpr::status::HTTP_BAD_REQUEST)
        {
            if (json["reason"].s() == "BUSY")
            {
                CROW_LOG_INFO << "Reintentando pago.";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                iniciar_pago(denom);
            }
        }
        else
        {
            CROW_LOG_CRITICAL << device_id << " → " << json["dispenseResult"].s() << ", Razon: " << json["dispenseResult"].s();
            signal_error.emit(device_id, json["dispenseResult"].operator std::string() + ", Razon: " + json["dispenseResult"].operator std::string());
            detiene_desconecta();
        }
    }
    catch (const std::exception &e)
    {
        CROW_LOG_ERROR << device_id << " → " << e.what();
    }
}

crow::json::wvalue ValidadorUnit::obten_cambio(uint &cambio, std::map<int, int> &reciclador, bool is_cambio)
{
    std::vector<int> billsToReturn(reciclador.size(), 0); // Vector para almacenar la cantidad de billetes a devolver
    auto cambio_original = cambio;                        // Guardamos el valor original del cambio
    int index = reciclador.size() - 1;                    // Índice para insertar en el vector
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
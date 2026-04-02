#include "core/hub_cash.hpp"

std::string CashHub::autentica()
{
    auto r = cpr::Post(cpr::Url{"http://localhost:5000/api/Users/Authenticate"},
                       cpr::Header{{"Content-Type", "application/json"}},
                       cpr::Body{R"({
                        "Username": "admin",
                        "Password": "password"
                        })"});

    if (r.status_code == crow::status::OK)
    {
        auto rcb = crow::json::load(r.text);
        return rcb["token"].s();
    }
    else if (r.status_code == 0)
    {
        Global::System::showNotify("Servicio", "Servicio API REST no iniciado o en error", "dialog-error");
        throw std::runtime_error("Servicio API REST no iniciado o en error");
    }
    else
        CROW_LOG_ERROR << "Error de autenticación con la API REST : " << r.status_code << " - " << r.text;

    return "";
}

#include <cstring> // Para memset

int CashHub::obtener_ssp_por_magia_negra(const std::string &puerto_dev, int ssp_address)
{
    CROW_LOG_INFO << "Intentando sincronizar SSP en dirección: " << ssp_address;

    SSP_PORT port_handle = OpenSSPPort(puerto_dev.c_str());
    if(port_handle == -1)
    {
        CROW_LOG_ERROR << "No se pudo abrir el puerto " << puerto_dev;
        return -1;
    }

    SSP_COMMAND_SETUP ssp_setup;
    std::memset(&ssp_setup, 0, sizeof(SSP_COMMAND_SETUP));

    ssp_setup.port = port_handle;
    ssp_setup.Timeout = 1000;
    ssp_setup.RetryLevel = 3;
    ssp_setup.SSPAddress = ssp_address;
    ssp_setup.EncryptionStatus = 0; 

    if (ssp_sync(ssp_setup) == SSP_RESPONSE_OK) {
        CROW_LOG_INFO << "¡Sincronización exitosa en dirección " << ssp_address << "!";
        return ssp_address;
    }

    if (ssp_address == 0) {
        CROW_LOG_WARNING << "Fallo en 0, intentando dirección 16...";
        CloseSSPPort(port_handle); 
        return obtener_ssp_por_magia_negra(puerto_dev, 16);
    }

    CROW_LOG_ERROR << "No se encontró ningún dispositivo SSP en " << puerto_dev;
    CloseSSPPort(port_handle);
    return -1;
}

// --- Escaneo Automático ---
void CashHub::inicializar_hardware()
{
    CROW_LOG_INFO << "Iniciando escaneo de puertos USB...";
    unidades.clear();
    bool ultimo_registro;

    for (const auto &entry : fs::directory_iterator("/dev"))
    {
        if (std::string path = entry.path().string(); path.find("/dev/ttyUSB") != std::string::npos)
        {
            int ssp_sugerido = obtener_ssp_por_magia_negra(path);
            if (ssp_sugerido != -1)
            {
                CROW_LOG_WARNING << "Puerto " << path << " detectado como Validador tipo "
                                 << (ssp_sugerido == 16 ? "COIN" : "Bill")
                                 << ". Usando SSP " << ssp_sugerido;
                intentar_registrar(path, ssp_sugerido);
            }
            else
            {
                Global::System::showNotify("Init System", ("No se pudo registra el validador con puerto " + path).c_str(), "dialog-information");
            }
        }
    }
}

bool CashHub::intentar_registrar(const std::string &puerto, int ssp)
{
    auto v = std::make_unique<ValidadorUnit>();

    v->property_token() = autentica();

    v->property_conf().puerto = puerto;
    v->property_conf().ssp = ssp;
    v->property_conf().habilita_recolector = false;
    v->property_conf().auto_acepta_credito = false;
    v->property_conf().habilita_salida_credito = false;

    // Intentar la conexión (OpenConnection)
    if (v->inicia_conecta(crow::json::load("[]")))
    {
        // ÉXITO: Conectar señales locales del objeto a las globales del HUB
        v->signal_event_received.connect([this,&v](std::string device_id, std::string type_val, std::string tipo, const crow::json::rvalue &data)
        {
            if (tipo == "STACKED" || tipo == "VALUE_ADDED" || tipo == "COIN_CREDIT" || tipo == "ESCROW") 
            {
                int monto = data["value"].i() / 100;
                signal_credito.emit(device_id, type_val, data, monto);
            } 
        });
        
        v->signal_error.connect([this](std::string device_id, std::string error_msg)
        {
            signal_hub_error.emit(device_id, error_msg);
        });
            
        std::this_thread::sleep_for(std::chrono::seconds(1));
        v->detiene_desconecta();

        unidades.push_back(std::move(v));
        return true;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    v->detiene_desconecta();

    return false;
}

crow::json::rvalue CashHub::rutas_default(ValidadorUnit *val)
{
    if (val->property_conf().ssp == 16)
        return crow::json::load("[]");

    crow::json::wvalue json_rutas; 
    auto m_list = std::make_unique<LevelCash>("Level_Bill")->get_level_cash();
    auto snapshot_level = crow::json::load(val->property_ultimo_cash_level());

    for (size_t i = 0; i < snapshot_level.size(); i++)
    {
        if (!snapshot_level[i].has("storedInPayout") || !snapshot_level[i].has("value")) continue;

        int valor = snapshot_level[i]["value"].i();
        std::string codigo = snapshot_level[i]["countryCode"].s();
        int almacenado = snapshot_level[i]["storedInPayout"].i();

        json_rutas["SetRoutes"][i]["Denomination"] = std::to_string(valor) + " " + codigo;
        
        // Lógica de ruta (1 = Recycler, 0 = Cashbox)
        bool es_payout = (almacenado <= m_list->get_item(i)->m_nivel_inmo_max);
        json_rutas["SetRoutes"][i]["Route"] = es_payout ? 1 : 0;

        CROW_LOG_DEBUG << "Cuadrando rutas: " << valor << " (Status: " << (es_payout ? "Recycler" : "Cashbox") << ")";
    }

    return crow::json::load(json_rutas.dump());
}

cpr::Response CashHub::command_by_device_id(HttpMethod method, const std::string &device_id, const std::string &command, const std::string &json, bool debug)
{
    for (auto const &i : unidades)
    {
        if (i->property_device_id() == device_id)
        {
            switch (method)
            {
            case HttpMethod::GET:
                return i->command_get(command, debug);
            case HttpMethod::POST:
                return i->command_post(command, json, debug);
            default:
                return {};
            }
        }
    }

    // Si llegamos aquí, el ID no existe.
    // Creamos una respuesta con error 404 local o status 0 para indicar "No encontrado"
    cpr::Response error_res;
    error_res.status_code = 0;
    error_res.error.message = "Dispositivo " + device_id + " no encontrado en el Hub";
    return error_res;
}

std::map<std::string, crow::json::rvalue> CashHub::obten_ultimo_snapshot_level(void)
{
    std::map<std::string, crow::json::rvalue> respuestas;

    for (auto &&i : unidades)
    {
        auto snapshot_original = crow::json::load(i->property_ultimo_cash_level());
        crow::json::wvalue envoltorio;
        envoltorio["levels"] = snapshot_original;
        envoltorio["type"] = (i->property_conf().ssp == 16 ? "COIN" : "BILL");
        respuestas[i->property_device_id()] = crow::json::load(envoltorio.dump());
    }

    return respuestas;
}

std::map<std::string, cpr::Response> CashHub::command_for_all(HttpMethod method, const std::string &command, const std::string &json, bool debug)
{
    if (command == "OpenConnection")
        throw "Usar la funcion 'inicia_for_all' en su lugar";

    std::map<std::string, cpr::Response> respuestas;

    for (auto &&i : unidades)
    {
        switch (method)
        {
        case HttpMethod::GET:
            respuestas[i->property_device_id()] = i->command_get(command, debug);
            break;
        case HttpMethod::POST:
            respuestas[i->property_device_id()] = i->command_post(command, json, debug);
            break;
        default:
            break;
        }
    }

    return respuestas;
}

void CashHub::inicia_for_all(const Conf &conf, std::map<std::string, const crow::json::rvalue> set_routes)
{
    for (auto &&i : unidades)
    {
        // por ahora todos tiene la misma configuracion de arranque
        i->property_token() = Sesion::token;
        i->property_conf().habilita_recolector = conf.habilita_recolector;
        i->property_conf().habilita_salida_credito = conf.habilita_salida_credito;
        i->property_conf().auto_acepta_credito = conf.auto_acepta_credito;

        if (auto device_id = i->property_device_id(); set_routes.contains(device_id))
            i->inicia_conecta(set_routes[device_id]);
        else
        {
            CROW_LOG_WARNING << "No se especificaron rutas para el dispositivo: " << device_id << " Usando las de default.";
            i->inicia_conecta(rutas_default(i.get()));
        }
    }
}

void CashHub::detiene_for_all(void)
{
    for (auto &&i : unidades)
        i->detiene_desconecta();
}

void CashHub::inicia_poll_for_all()
{
    for (auto &&i : unidades)
        i->iniciar_polling();
}

void CashHub::detiene_poll_for_all(size_t t_id)
{
    std::vector<cpr::Response> snapshot_inicio;
    std::vector<cpr::Response> snapshot_fin;

    for (auto &&i : unidades)
    {
        cpr::Response r_inicio;
        r_inicio.text = i->property_ultimo_cash_level();
        r_inicio.status_code = 200;
        snapshot_inicio.emplace_back(r_inicio);
        i->property_poll().store(false);
        auto json = i->command_get("GetAllLevels", true);
        i->property_ultimo_cash_level() = json.text;
        snapshot_fin.emplace_back(json);
    }

    for (size_t i = 0; i < snapshot_inicio.size(); i++)
        detalle.registrar_diferencias(t_id, 
            crow::json::load(snapshot_inicio[i].text), 
            crow::json::load(snapshot_fin[i].text));
        
}

// automatico
void CashHub::inicia_pago(size_t t_id, size_t monto, bool is_cambio)
{
    uint remanente = monto;
    std::vector<cpr::Response> snapshot_inicio;
    std::vector<cpr::Response> snapshot_fin;

    for (auto &&i : unidades)
    {
        if (i->property_conf().ssp == 0)
        {
            cpr::Response r_inicio;
            r_inicio.text = i->property_ultimo_cash_level();
            auto json_response = crow::json::load(r_inicio.text);
            r_inicio.status_code = 200;
            snapshot_inicio.emplace_back(r_inicio);
            CROW_LOG_INFO << "Intentando pagar con Billetes...";
            remanente = i->iniciar_pago(remanente, is_cambio, json_response);
            snapshot_fin.emplace_back(i->command_get("GetAllLevels", true));
        }
    }

    if (remanente > 0)
    {
        for (auto &&i : unidades)
        {
            if (i->property_conf().ssp == 16)
            {
                auto response = i->command_get("GetAllLevels", true);
                auto json_response = crow::json::load(response.text);
                snapshot_inicio.emplace_back(response);
                CROW_LOG_INFO << "Intentando pagar resto con Monedas...";
                remanente = i->iniciar_pago(remanente, is_cambio, json_response);
                snapshot_fin.emplace_back(i->command_get("GetAllLevels", true));
            }
        }
    }

    for (size_t i = 0; i < snapshot_inicio.size(); i++)
        detalle.registrar_diferencias(t_id, 
            crow::json::load(snapshot_inicio[i].text), 
            crow::json::load(snapshot_fin[i].text));

    if (remanente > 0)
        signal_hub_error.emit("General", "Cambio incompleto. Faltaron: " + std::to_string(remanente));
}

// manual
void CashHub::inicia_pago(size_t t_id, std::map<std::string, std::string> map)
{
    std::vector<cpr::Response> snapshot_inicio;
    std::vector<cpr::Response> snapshot_fin;

    for (auto &&i : unidades)
        if (map.contains(i->property_device_id()))
        {
            snapshot_inicio.emplace_back(i->command_get("GetAllLevels", true));
            i->iniciar_pago(map.at(i->property_device_id()));
            snapshot_fin.emplace_back(i->command_get("GetAllLevels", true));
        }
        else
            CROW_LOG_ERROR << "No se encontro el dispositivo " << i->property_device_id() << ", Para pago manual";
    
    for (size_t i = 0; i < snapshot_inicio.size(); i++)
        detalle.registrar_diferencias(t_id, 
            crow::json::load(snapshot_inicio[i].text), 
            crow::json::load(snapshot_fin[i].text));
}
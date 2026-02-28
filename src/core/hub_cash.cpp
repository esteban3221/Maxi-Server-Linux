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
        throw std::runtime_error("Servicio API REST no iniciado o en error");
    else
        CROW_LOG_ERROR << "Error de autenticación con la API REST : " << r.status_code << " - " << r.text;

    return "";
}

int CashHub::obtener_ssp_por_serial(const std::string &puerto_dev)
{
    std::string nombre_tty = puerto_dev.substr(puerto_dev.find_last_of('/') + 1); // "ttyUSB1"
    std::string ruta_serial = "/sys/class/tty/" + nombre_tty + "/device/../../serial";

    std::ifstream archivo(ruta_serial);
    std::string serial;

    if (archivo >> serial)
    {
        if (serial == "A10L8NSY")
        {
            CROW_LOG_INFO << "Hardware identificado: Smart Coin System (SSP 16)";
            return 16;
        }
        if (serial == "A10L8NNB")
        {
            CROW_LOG_INFO << "Hardware identificado: Spectral Payout (SSP 0)";
            return 0;
        }
    }

    CROW_LOG_WARNING << "No se pudo identificar el serial. Usando SSP 0 por defecto.";
    return 0;
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
            int ssp_sugerido = obtener_ssp_por_serial(path);
            if (ssp_sugerido != -1)
            {
                CROW_LOG_WARNING << "Puerto " << path << " detectado como Serial "
                                 << (ssp_sugerido == 16 ? "A10L8NSY" : "A10L8NNB")
                                 << ". Usando SSP " << ssp_sugerido;
                intentar_registrar(path, ssp_sugerido);
            }
            else
            {
                // Si conectas un hardware nuevo, aquí sí puedes probar ambos
                intentar_registrar(path, 0);
                intentar_registrar(path, 16);
            }

            // std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
}

bool CashHub::intentar_registrar(const std::string &puerto, int ssp)
{
    //CROW_LOG_INFO << "Puerto detectado: " << puerto << ". Probando perfil SSP " << ssp;
    auto v = std::make_unique<ValidadorUnit>();

    v->property_token() = autentica();

    v->property_conf().puerto = puerto;
    v->property_conf().ssp = ssp;
    v->property_conf().habilita_recolector = true;
    v->property_conf().auto_acepta_credito = true;
    v->property_conf().habilita_salida_credito = true;

    // Intentar la conexión (OpenConnection)
    auto response = v->inicia_conecta(crow::json::load("[]"));

    if (response)
    {
        // ÉXITO: Conectar señales locales del objeto a las globales del HUB
        v->signal_event_received.connect([this](std::string tipo, const crow::json::rvalue &data)
                                         {
            if (tipo == "STACKED" || tipo == "VALUE_ADDED" || tipo == "COIN_CREDIT") 
            {
                int monto = data["value"].i() / 100;
                signal_credito.emit(data, monto);
            } });
            
        std::this_thread::sleep_for(std::chrono::seconds(1));
        v->detiene_desconecta();

        unidades.push_back(std::move(v));
        return true;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    v->detiene_desconecta();

    return false;
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
            respuestas[i->property_device_model()] = i->command_get(command, debug);
            break;
        case HttpMethod::POST:
            respuestas[i->property_device_model()] = i->command_post(command, json, debug);
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
        i->property_conf().habilita_recolector = conf.habilita_recolector;
        i->property_conf().habilita_salida_credito = conf.habilita_salida_credito;
        i->property_conf().auto_acepta_credito = conf.auto_acepta_credito;

        if (auto modelo = i->property_device_model(); set_routes.contains(modelo))
            i->inicia_conecta(set_routes[modelo]);
        else
        {
            CROW_LOG_WARNING << "No se encontraron rutas para el modelo: " << modelo;
            i->inicia_conecta(crow::json::load("[]"));
        }
    }
}

void CashHub::detiene_for_all(void)
{
    for (auto &&i : unidades) i->detiene_desconecta();
}

void CashHub::inicia_poll_for_all()
{
    for (auto &&i : unidades) i->iniciar_polling();
}

void CashHub::detiene_poll_for_all()
{
    for (auto &&i : unidades) i->property_poll().store(false);
}

void CashHub::inicia_pago(size_t monto)
{
    uint remanente = monto;

    // 1. Primero buscamos los validadores de billetes (SSP 0 por convención ITL)
    for (auto &&i : unidades)
    {
        if (i->property_conf().ssp == 0) { 
            CROW_LOG_INFO << "Intentando pagar con Billetes...";
            remanente = i->iniciar_pago(remanente);
        }
    }

    // 2. Lo que sobre (remanente), intentamos pagarlo con monedas (SSP 16)
    if (remanente > 0) {
        for (auto &&i : unidades)
        {
            if (i->property_conf().ssp == 16) { 
                CROW_LOG_INFO << "Intentando pagar resto con Monedas...";
                remanente = i->iniciar_pago(remanente);
            }
        }
    }

    if (remanente > 0) 
        signal_hub_error.emit("Cambio incompleto. Faltaron: " + std::to_string(remanente));
}
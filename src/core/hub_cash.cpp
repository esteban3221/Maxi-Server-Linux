#include "core/hub_cash.hpp"

std::string CashHub::autentica()
{
    auto r = cpr::Post(cpr::Url{"http://localhost:5000/api/Users/Authenticate"},
                       cpr::Header{{"Content-Type", "application/json"} /*,
                                    {"Authorization", "Bearer " + token}*/
                       },
                       cpr::Body{R"({
                        "Username": "admin",
                        "Password": "password"
                        })"});

    if (r.status_code == crow::status::OK)
    {
        auto rcb = crow::json::load(r.text);
        return rcb["token"].s();
    }
    else
        CROW_LOG_ERROR << "Error de autenticación: " << r.status_code << " - " << r.text;

    return "";
}

// --- Escaneo Automático ---
void CashHub::inicializar_hardware()
{
    CROW_LOG_INFO << "Iniciando escaneo de puertos USB...";

    for (const auto &entry : fs::directory_iterator("/dev"))
    {
        std::string path = entry.path().string();
        if (path.find("/dev/ttyUSB") != std::string::npos)
        {
            CROW_LOG_INFO << "Puerto detectado: " << path << ". Probando perfiles...";

            // Intentamos primero como Billetes (SSP 0)
            if (!intentar_registrar(path, 0))
            {
                // Si falla, intentamos como Monedas (SSP 16)
                intentar_registrar(path, 16);
            }
        }
    }
}

bool CashHub::intentar_registrar(const std::string &puerto, int ssp)
{
    auto v = std::make_unique<ValidadorUnit>();

    v->property_token() = autentica();
    Conf config;
    config.puerto = puerto;
    config.ssp = ssp;
    config.habilita_recolector = false;
    config.auto_acepta_credito = false;
    config.habilita_salida_credito = false;

    // Intentar la conexión (OpenConnection)
    auto response = v->inicia_conecta(config, crow::json::load("[]"));

    if (response)
    {
        // ÉXITO: Conectar señales locales del objeto a las globales del HUB
        v->signal_event_received.connect([this, ptr = v.get(), ssp](std::string tipo, const crow::json::rvalue &data)
                                         {
                if (tipo == "STACKED" || tipo == "VALUE_ADDED" || tipo == "COIN_CREDIT") {
                    int monto = data["value"].i();
                    (ssp == 0) ? signal_credito_billete.emit(ptr, monto) :
                                 signal_credito_moneda.emit(ptr, monto);
                } });

        v->iniciar_polling();
        unidades.push_back(std::move(v));
        return true;
    }
    return false;
}

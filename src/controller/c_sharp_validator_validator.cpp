#include "c_sharp_validator.hpp"

namespace Global
{
    namespace Device
    {
        Validator dv_coin("HOPPER"), dv_bill("SPECTRAL");
    } // namespace Device
}

Validator::Validator(const std::string &validator, const std::source_location location)
    : validator(validator)
{
    ++instance_count;

    if (instance_count > 2)
        std::cout << YELLOW << "Advertencia: " << RESET << "Mas de 2 instancias de validadores detectadas, Posible fuga? \n"
                  << WHITE << "Ruta: " << RESET << location.file_name() << '(' << location.line() << ':' << location.column() << ") \n";
}

Validator::~Validator()
{
    --instance_count;
}

void Validator::imprime_debug(int status, const std::string &comando, const std::string &body) const
{
    std::cout << BOLDBLACK << "======== DEBUG ========\n"
              << BOLDBLACK << "Comando: " << WHITE << comando << '\n'
              << BOLDBLACK << "Validador: " << WHITE << validator << '\n'
              << BOLDBLACK << "Responde Code: " << (status != crow::status::OK ? RED : GREEN) << status << '\n'
              << BOLDBLACK << "Body: " << WHITE << body << '\n'
              << RESET;
}

std::pair<int, std::string> Validator::command_post(const std::string &command, const std::string &json, bool debug)
{
    r_ = cpr::Post(cpr::Url{Global::ApiConsume::BASE_URL + "/" + command + "?deviceID=" + validator},
                   cpr::Header{{"Content-Type", "application/json"},
                               {"Authorization", "Bearer " + Global::ApiConsume::token}},
                   cpr::Body{json});

    if (debug)
        imprime_debug(r_.status_code, command, r_.text);

    return {r_.status_code, r_.text};
}

int Validator::reintenta_comando_post(const std::string &comando, const std::string &datos, int &intentos)
{
    const int max_intentos = 5;
    int status;
    do
    {
        status = command_post(comando, datos, true).first;

        if (status != crow::status::OK)
        {
            std::cout << "Estado de " << comando << " no OK. Reintentando... (" << intentos + 1 << "/" << max_intentos << ")" << std::endl;
            intentos++;
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Espera 500 ms antes de reintentar
        }

        if (intentos >= max_intentos)
        {
            std::cerr << "Número máximo de intentos alcanzado para " << comando << ". Abortando..." << std::endl;
            break;
        }

    } while (status != crow::status::OK);

    return status;
}

std::pair<int, std::string> Validator::command_get(const std::string &command, bool debug) const
{
    const auto r = cpr::Get(cpr::Url{Global::ApiConsume::BASE_URL + "/" + command + "?deviceID=" + validator},
                            cpr::Header{{"Content-Type", "application/json"},
                                        {"Authorization", "Bearer " + Global::ApiConsume::token}});

    if (debug)
        imprime_debug(r.status_code, command, r.text);

    return {r.status_code, r.text};
}

void Validator::poll(const std::function<void(const std::string &, const crow::json::rvalue &)> &func)
{
    Global::EValidador::balance.cambio.store(0);

    while (Global::EValidador::is_running.load() /*|| (Global::EValidador::balance.ingreso.load() >= Global::EValidador::balance.total.load())*/)
    {
        std::lock_guard<std::mutex> lock(poll_mutedx);
        /*Se queda con una cola de eventos y de vez en cuando retiene dinero logicamente hasta que se vuelve a consultar*/
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
        auto status = command_get("GetDeviceStatus");

        if (status.first == crow::status::OK)
        {
            /**/
            auto json_data = crow::json::load(status.second);
            for (const auto &i : json_data)
            {
                std::string state;

                if (i.has("stateAsString"))
                    state = i["stateAsString"].s();
                else if (i.has("eventTypeAsString"))
                    state = i["eventTypeAsString"].s();

                func(state, i);
            }
        }
        else
            break;
    }
}

void Validator::init(Global::EValidador::Conf conf)
{
    command_post("InitialiseDevice");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    crow::json::wvalue json;

    json["ComPort"] = conf.puerto;
    json["SSPAddress"] = conf.ssp;
    json["Name"] = conf.dispositivo = validator;

    command_post("OpenDevice", json.dump(), true);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

/*
http://localhost:5000/api/CashDevice/OpenConnection
{
  "ComPort": "/dev/ttyACM0",
  "SspAddress": 0,
  "LogFilePath": "/home/hang-phan/DeviceLog.log",
  "SetInhibits": [
    {
      "Denomination": "10000 USD",
      "Inhibit": true
    },
    {
      "Denomination": "5000 USD",
      "Inhibit": true
    }
  ],
  "SetRoutes": [
    {
      "Denomination": "100 USD",
      "Route": 7
    },
    {
      "Denomination": "500 USD",
      "Route": 7
    },
    {
      "Denomination": "1000 USD",
      "Route": 7
    }
  ],
  "EnableAcceptor": true,
  "EnableAutoAcceptEscrow": true,
  "EnablePayout": true
}*/

/*
{
    "deviceID": "SMART_COIN_SYSTEM-/dev/ttyUSB0",
    "isOpen": true,
    "deviceModel": "SMART_COIN_SYSTEM",
    "sspProtocolVersion": 7,
    "deviceError": "NONE",
    "firmware": "SH00041312579C01",
    "dataset": "MXN01051",
    "primaryHopperSerialNumber": "5739234",
    "coinFeederSerialNumber": "5725983",
    "primaryHopperBuildRevision": "13.00",
    "coinFeederBuildRevision": "13.00",
    "realTimeClock": "Successfully set real time clock to 09/01/2025 11:45:30 a.m..",
    "allLevels": [
        {
            "countryCode": "MXN",
            "value": 100,
            "stored": 16
        },
        {
            "countryCode": "MXN",
            "value": 200,
            "stored": 4
        },
        {
            "countryCode": "MXN",
            "value": 500,
            "stored": 0
        },
        {
            "countryCode": "MXN",
            "value": 1000,
            "stored": 0
        }
    ],
    "counters": "Number of counters in set: 9\nCoins paid out, including to cashbox: 1052\nCoins paid in: 1156\nFeeder rejects: 56\nHopper jams: 0\nFeeder jams: 0\nFraud attempts: 1\nCalibration Fails: 0\nResets: 169\nCoins sent to cashbox: 459\n",
    "acceptorEnabled": true
}
*/

crow::json::rvalue Validator::inicia_dispositivo_v8(const Global::EValidador::Conf &conf)
{
    crow::json::wvalue data_in =
        {
            {"ComPort", conf.puerto},
            {"SspAddress", conf.ssp},
            {"LogFilePath", conf.log_ruta},
            {"EnableAcceptor", conf.habilita_recolector},
            {"EnablePayout", conf.habilita_recolector},
            {"EnableAutoAcceptEscrow", conf.auto_acepta_billetes}};
    this->conf = conf;
    auto data_out = command_post("OpenConnection", data_in.dump());

    auto json = crow::json::load(data_out.second);
    this->validator = data_out.first == crow::status::OK ? std::string(json["deviceID"].s()) : validator;

    return json;
}

void Validator::deten_cobro_v8()
{
}

const Global::EValidador::Conf &Validator::get_data_conf()
{
    return this->conf;
}

const crow::json::rvalue &Validator::get_status_coneccion()
{
    return this->json_data_status_coneccion;
}

void Validator::inicia_dispositivo_v6()
{
    int intentos_start = 0, intentos_enable_payout = 0, intentos_enable_acceptor = 0;
    if (reintenta_comando_post("StartDevice", "", intentos_start) != crow::status::OK)
    {
        return; // Si falla, sal de la función
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    // Ejecuta "EnablePayout" con reintentos
    if (reintenta_comando_post("EnablePayout", "", intentos_enable_payout) != crow::status::OK)
    {
        return; // Si falla, sal de la función
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    // Ejecuta "EnableAcceptor" con reintentos
    if (reintenta_comando_post("EnableAcceptor", "", intentos_enable_acceptor) != crow::status::OK)
    {
        return; // Si falla, sal de la función
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    // }

    // command_post("StartDevice", "", true);
    // std::this_thread::sleep_for(std::chrono::milliseconds(600));
    // command_post("EnablePayout", "", true);
    // std::this_thread::sleep_for(std::chrono::milliseconds(400));
    // command_post("EnableAcceptor", "", true);
    // std::this_thread::sleep_for(std::chrono::milliseconds(900));
}

void Validator::deten_cobro_v6()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(900));

    command_post("HaltPayout", "", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    command_post("DisableAcceptor", "", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    command_post("DisablePayout", "", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void Validator::acepta_dinero(const std::string &state, bool recy)
{
    if (state == "ESCROW")
    {
        crow::json::wvalue json;

        json["Value"] = Global::EValidador::balance.ingreso_parcial;
        json["CountryCode"] = "MXN";
        json["Route"] = (int)recy;

        command_post("SetDenominationRoute", json.dump(), true);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        command_post("AcceptFromEscrow");
    }
}

Glib::RefPtr<Gio::ListStore<MLevelCash>> Validator::get_level_cash_actual() const
{
    auto m_list = Gio::ListStore<MLevelCash>::create();
    auto json_string = command_get("GetAllLevels").second;
    auto json = crow::json::load(json_string);

    for (auto &&i : json["levels"])
    {
        m_list->append(MLevelCash::create(
            i["value"].i(),  // denomonacion
            0,               // cassete
            i["stored"].i(), // i["value"].i(), // recyclado
            0,               // i["value"].i() // tope dinero
            0));
    }
    return m_list;
}

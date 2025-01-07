#include "validator.hpp"

Validator::Validator(const std::string &validator) : VALIDATOR(validator) , ingreso_parcial(0)
{
}

Validator::~Validator()
{
}

void Validator::imprime_debug(int status, const std::string &body)
{
    std::cout << BOLDBLACK << "======== DEBUG ========"
              << BOLDBLACK << "Responde Code: " << (status != crow::status::OK ? RED : GREEN) << status << '\n'
              << BOLDBLACK << "Body: " << WHITE << body << '\n';
}

std::pair<int, std::string> Validator::command_post(const std::string &command, const std::string &json = "", bool debug = false)
{
    r_ = cpr::Post(cpr::Url{Global::ApiConsume::BASE_URL + "/" + command + "?deviceID=" + VALIDATOR},
                   cpr::Header{{"Content-Type", "application/json"},
                               {"Authorization", "Bearer " + Global::ApiConsume::token}},
                   cpr::Body{json});

    if (debug)
        imprime_debug(r_.status_code, r_.text);

    return {r_.status_code, r_.text};
}

std::pair<int, std::string> Validator::command_get(const std::string &command, bool debug = false)
{
    r_ = cpr::Get(cpr::Url{Global::ApiConsume::BASE_URL + "/" + command + "?deviceID=" + VALIDATOR},
                  cpr::Header{{"Content-Type", "application/json"},
                              {"Authorization", "Bearer " + Global::ApiConsume::token}});

    if (debug)
        imprime_debug(r_.status_code, r_.text);

    return {r_.status_code, r_.text};
}

void Validator::poll(std::function<void(const std::string &, const crow::json::rvalue &)> &func)
{
    Global::EValidador::balance.ingreso.store(0);
    Global::EValidador::balance.total.store(0);
    Global::EValidador::balance.cambio.store(0);

    while (Global::EValidador::is_running.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (auto status = command_get("GetDeviceStatus"); status.first == crow::status::OK)
        {
            std::cout << " TEST: " << status.second << std::endl;
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
    json["Name"] = conf.nombre = VALIDATOR;

    command_post("OpenDevice", json.dump(), true);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void Validator::inicia_cobro()
{
    command_post("ConnectDevice", "", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    command_post("StartDevice", "", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void Validator::deten_cobro()
{
    command_post("HaltPayout", "", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    command_post("DisableAcceptor", "", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    command_post("DisablePayout", "", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void Validator::acepta_dinero(const std::string &state, bool recy = false)
{
    if (state == "ESCROW")
    {
        crow::json::wvalue json;

        json["Value"] = ingreso_parcial;
        json["CountryCode"] = "MXN";
        json["Route"] = (int)recy;

        command_post("SetDenominationRoute", json.dump(), true);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        command_post("AcceptFromEscrow");
    }
}

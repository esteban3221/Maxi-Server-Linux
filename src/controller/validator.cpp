#include "validator.hpp"

Validator::Validator(const std::string &validator) : VALIDATOR(validator)
{
}

Validator::~Validator()
{
}

std::pair<int, std::string> Validator::Command_Post(const std::string &command)
{
    r = cpr::Post(cpr::Url{Global::Api_consume::BASE_URL + "/" + command + "?deviceID=" + VALIDATOR},
                  cpr::Header{{"Content-Type", "application/json"},
                              {"Authorization", "Bearer " + Global::Api_consume::token}});

    return {r.status_code, r.text};
}

std::pair<int, std::string> Validator::Command_Post(const std::string &command, const std::string &json)
{
    r = cpr::Post(cpr::Url{Global::Api_consume::BASE_URL + "/" + command + "?deviceID=" + VALIDATOR},
                  cpr::Header{{"Content-Type", "application/json"},
                              {"Authorization", "Bearer " + Global::Api_consume::token}},
                  cpr::Body{json});

    return {r.status_code, r.text};
}

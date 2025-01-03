#include "validator.hpp"

Validator::Validator(const std::string &validator) : VALIDATOR(validator)
{
}

Validator::~Validator()
{
}

std::pair<int, std::string> Validator::Command_Post(const std::string &command)
{
    r_ = cpr::Post(cpr::Url{Global::Api_consume::BASE_URL + "/" + command + "?deviceID=" + VALIDATOR},
                  cpr::Header{{"Content-Type", "application/json"},
                              {"Authorization", "Bearer " + Global::Api_consume::token}});

    return {r_.status_code, r_.text};
}

std::pair<int, std::string> Validator::Command_Get(const std::string &command)
{
    r_ = cpr::Get(cpr::Url{Global::Api_consume::BASE_URL + "/" + command + "?deviceID=" + VALIDATOR},
                  cpr::Header{{"Content-Type", "application/json"},
                              {"Authorization", "Bearer " + Global::Api_consume::token}});

    return {r_.status_code, r_.text};
}

std::pair<int, std::string> Validator::Command_Post(const std::string &command, const std::string &json)
{
    r_ = cpr::Post(cpr::Url{Global::Api_consume::BASE_URL + "/" + command + "?deviceID=" + VALIDATOR},
                  cpr::Header{{"Content-Type", "application/json"},
                              {"Authorization", "Bearer " + Global::Api_consume::token}},
                  cpr::Body{json});

    return {r_.status_code, r_.text};
}

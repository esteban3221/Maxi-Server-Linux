#pragma once
#include <utility>
#include <iostream>
#include <crow.h>
#include <cpr/cpr.h>
#include <functional>
#include "global.hpp"

class Validator
{
private:
    cpr::Response r_;
    const std::string VALIDATOR;

    uint32_t ingreso_parcial;

    void imprime_debug(int status, const std::string &body);

public:
    std::pair<int, std::string> command_get(const std::string &command, bool debug);
    std::pair<int, std::string> command_post(const std::string &command, const std::string &json, bool);

    void poll(std::function<void(const std::string &, const crow::json::rvalue &)> &func);
    void inicia_cobro();
    void deten_cobro();
    void init(Global::EValidador::Conf conf);
    void acepta_dinero(const std::string &state, bool recy);

    Validator(const std::string &validator);
    ~Validator();
};

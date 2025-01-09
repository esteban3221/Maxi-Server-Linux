#pragma once
#include <utility>
#include <iostream>
#include <crow.h>
#include <cpr/cpr.h>
#include <functional>
#include <source_location>

#include "global.hpp"
#include "level_cash.hpp"

class Validator
{
private:
    static inline int instance_count;
    cpr::Response r_;
    std::string validator;

    uint32_t ingreso_parcial;

    void imprime_debug(int status, const std::string &comando, const std::string &body);

public:
    Validator(const Validator &) = delete;
    Validator &operator=(const Validator &) = delete;

    std::pair<int, std::string> command_get(const std::string &command, bool debug);
    std::pair<int, std::string> command_post(const std::string &command, const std::string &json, bool);

    void poll(std::function<void(const std::string &, const crow::json::rvalue &)> &func);

    //version con net 6
    void inicia_dispositivo_v6();
    void deten_cobro_v6();

    //version con net 8
    crow::json::rvalue inicia_dispositivo_v8(Global::EValidador::Conf &conf);
    void deten_cobro_v8();

    void init(Global::EValidador::Conf conf);
    void acepta_dinero(const std::string &state, bool recy);

    Glib::RefPtr<Gio::ListStore<MLevelCash>> get_level_cash_actual();

    Validator(const std::string &validator, const std::source_location location = std::source_location::current());
    ~Validator();
};

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

    crow::json::rvalue json_data_status_coneccion;

    uint32_t ingreso_parcial;
    Global::EValidador::Conf conf;

    void imprime_debug(int status, const std::string &comando, const std::string &body) const;

public:
    Validator(const Validator &) = delete;
    Validator &operator=(const Validator &) = delete;

    std::pair<int, std::string> command_get(const std::string &command, bool debug = false) const;
    std::pair<int, std::string> command_post(const std::string &command, const std::string &json = "", bool = false);

    void poll(const std::function<void(const std::string &, const crow::json::rvalue &)> &func);

    //version con net 6
    void inicia_dispositivo_v6();
    void deten_cobro_v6();

    //version con net 8
    crow::json::rvalue inicia_dispositivo_v8(const Global::EValidador::Conf &conf);
    void deten_cobro_v8();

    const Global::EValidador::Conf &get_data_conf();
    const crow::json::rvalue &get_status_coneccion();

    void init(Global::EValidador::Conf conf);
    void acepta_dinero(const std::string &state, bool recy = false);

    Glib::RefPtr<Gio::ListStore<MLevelCash>> get_level_cash_actual() const;

    Validator(const std::string &validator, const std::source_location location = std::source_location::current());
    ~Validator();
};


namespace Global
{
    namespace Device
    {
        extern Validator dv_coin ,dv_bill;
    } // namespace Device
}
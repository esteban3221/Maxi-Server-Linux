#pragma once
#include <utility>
#include <iostream>
#include <crow.h>
#include <cpr/cpr.h>
#include <functional>
#include <source_location>
#include <mutex>

#include "global.hpp"
#include "level_cash.hpp"
#include "model/level_cash.hpp"

class Validator
{
private:
    static inline int instance_count;
    cpr::Response r_;
    std::string validator;
    std::mutex poll_mutedx;

    crow::json::rvalue json_data_status_coneccion;

    Global::EValidador::Conf conf;

    void imprime_debug(const cpr::Response &r, const std::string &comando) const;

public:
    Validator(const Validator &) = delete;
    Validator &operator=(const Validator &) = delete;

    std::pair<int, std::string> command_get(const std::string &command, bool debug = false) const;
    std::pair<int, std::string> command_post(const std::string &command, const std::string &json = "", bool = false);
    std::pair<int, std::string> reintenta_comando_post(const std::string& comando, const std::string& datos, int& intentos);

    void poll(const std::function<void(const std::string &, const crow::json::rvalue &)> &func);

    //version con net 6
    void inicia_dispositivo_v6(bool auto_acepta_billetes = true);
    void deten_cobro_v6();

    //version con net 8
    crow::json::rvalue inicia_dispositivo_v8(const Global::EValidador::Conf &conf);
    void deten_cobro_v8();

    const Global::EValidador::Conf &get_data_conf();
    const crow::json::rvalue &get_status_coneccion();

    void init(Global::EValidador::Conf conf);
    void acepta_dinero(int deno, bool recy = false);

    Glib::RefPtr<Gio::ListStore<MLevelCash>> get_level_cash_actual(bool extendido = false, bool debug = false) const;

    std::atomic<bool> is_busy;

    Validator(const std::string &validator, const std::source_location location = std::source_location::current());
    ~Validator();
};



    namespace Device
    {
        extern Validator dv_coin ,dv_bill;
        extern std::map<int, int> map_cantidad_recyclador(const Validator &val);
    } // namespace Device

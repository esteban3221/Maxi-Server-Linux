#pragma once
#include "global.hpp"
#include "core/hub_cash.hpp"
#include "model/log.hpp"
#include "model/usuarios.hpp"
#include "model/detalle_movimiento.hpp"

class LogData
{
private:
    crow::response get_log(const crow::request &req);
    crow::response corte_caja(const crow::request &req);
    crow::response get_levels(const crow::request &req);

public:
    LogData(crow::SimpleApp& app);
    ~LogData();
};

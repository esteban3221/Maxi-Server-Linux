#pragma once
#include "global.hpp"
#include "model/log.hpp"
#include "model/usuarios.hpp"

namespace RestApp = Global::Rest;
class LogData
{
private:
    crow::response get_log(const crow::request &req);
    crow::response corte_caja(const crow::request &req);

public:
    LogData(/* args */);
    ~LogData();
};

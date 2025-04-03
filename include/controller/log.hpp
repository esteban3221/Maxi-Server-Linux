#pragma once
#include "global.hpp"
#include <glibmm.h>

namespace RestApp = Global::Rest;
class LogData
{
private:
    crow::response actualiza_impresion(const crow::request &req);
    crow::response actualiza_informacion(const crow::request &req);
    crow::response get_informacion(const crow::request &req);
    crow::response testImpresion(const crow::request &req);

    crow::response reiniciar(const crow::request &req);
    crow::response apagar(const crow::request &req);

    crow::response get_informacion_sistema(const crow::request &req);

public:
    LogData(/* args */);
    ~LogData();
};

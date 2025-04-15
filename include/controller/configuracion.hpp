#pragma once
#include "global.hpp"
#include "controller/config/impresora.hpp"
#include "controller/c_sharp_validator.hpp"
#include "model/configuracion.hpp"

namespace RestApp = Global::Rest;
class CConfiguracion
{
private:
crow::response actualiza_impresion(const crow::request &req);
crow::response actualiza_informacion_empresa(const crow::request &req);
crow::response get_informacion_empresa(const crow::request &req);
crow::response get_informacion_impresora(const crow::request &req);
crow::response test_impresion(const crow::request &req);

crow::response reiniciar(const crow::request &req);
crow::response apagar(const crow::request &req);

crow::response get_informacion_sistema(const crow::request &req);
public:
    CConfiguracion(/* args */);
    ~CConfiguracion();
};

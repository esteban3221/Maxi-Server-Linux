#pragma once
#include "global.hpp"
#include "controller/config/impresora.hpp"
#include "controller/c_sharp_validator.hpp"
#include "c_sharp_validator.hpp"
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
crow::response get_informacion_wifi(const crow::request &req);
crow::response actualiza_pos(const crow::request &req);

crow::response custom_command(const crow::request &req);

void descomprime_zip(const std::string &filename);
void limpiar_archivos(const std::string &filename, const std::string &path = "");
public:
    CConfiguracion(/* args */);
    ~CConfiguracion();
};

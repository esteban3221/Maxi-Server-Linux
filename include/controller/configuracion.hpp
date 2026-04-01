#pragma once
#include "global.hpp"
#include "controller/config/impresora.hpp"
#include "core/hub_cash.hpp"

#include "model/configuracion.hpp"
#include "model/log.hpp"
#include "carrousel.hpp"

class CConfiguracion
{
private:
crow::response actualiza_impresion(const crow::request &req);
crow::response actualiza_informacion_empresa(const crow::request &req);
crow::response actualiza_pos(const crow::request &req);
crow::response actualiza_operaciones(const crow::request &req);

crow::response get_informacion_empresa(const crow::request &req);
crow::response get_informacion_impresora(const crow::request &req);
crow::response get_informacion_operaciones(const crow::request &req);
crow::response get_informacion_sistema(const crow::request &req);
crow::response get_volcado_servicio(const crow::request &req);

crow::response test_impresion(const crow::request &req);
crow::response reiniciar(const crow::request &req);
crow::response apagar(const crow::request &req);

crow::response desactiva_carrousel(const crow::request &req);
crow::response sube_carpeta_pos(const crow::request &req);
crow::response sube_imagen_pos(const crow::request &req);

crow::response custom_command(const crow::request &req);
crow::response reinicia_validadores(const crow::request &req);


void descomprime_zip(const std::string &filename);
void limpiar_archivos(const std::string &filename, const std::string &path = "");
public:
    CConfiguracion(crow::SimpleApp& app);
    ~CConfiguracion();
};

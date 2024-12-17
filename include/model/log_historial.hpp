#pragma once
#include <vector>
#include <iostream>

struct log_historial_
{
    size_t id;
    int original_id;
    int old_id_user;
    std::string old_tipo;
    int old_ingreso;
    int old_cambio;
    int old_total;
    std::string old_estatus;
    std::string old_fecha;
    int new_id_user;
    std::string new_tipo;
    int new_ingreso;
    int new_cambio;
    int new_total;
    std::string new_estatus;
    std::string new_fecha;
    std::string fecha_modificacion;
    std::string usuario_operacion;
    std::string tipo_cambio;
};


class Log_historial
{
private:
    /* data */
public:
    Log_historial(/* args */);
    ~Log_historial();

    std::vector<log_historial_> get_log_historial();
};

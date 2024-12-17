#include "log.hpp"

Log::Log(/* args */)
{
}

Log::~Log()
{
}

std::vector<Log_> Log::get_log()
{
    std::vector<Log_> vec_log;

    for (size_t i = 0; i < 10; i++)
    {
        Log_ log;

        log.id = i;
        log.id_user = 1;
        log.tipo = "1";
        log.ingreso = 1;
        log.cambio = 1;
        log.total = 1;
        log.estatus = "1";
        log.fecha = "1";

        vec_log.push_back(log);
    }
    

    return vec_log;
}

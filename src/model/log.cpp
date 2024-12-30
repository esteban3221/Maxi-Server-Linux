#include "log.hpp"

Log::Log(/* args */)
{
}

Log::~Log()
{
}

std::vector<std::shared_ptr<Model::Log_t>> Log::get_log()
{
    std::vector<std::shared_ptr<Model::Log_t>> logVector;

    for (size_t i = 0; i < 10; i++)
    {
        auto log1 = std::make_shared<Model::Log_t>([&i](Model::Log_t &log)
        {
            log.id = i;
            log.id_user = 101;
            log.tipo = "venta";
            log.ingreso = 1000;
            log.cambio = 200;
            log.total = 800;
            log.estatus = "completo";
            log.fecha = "2024-12-30"; 
        });

        logVector.emplace_back(log1);
    }

    return logVector;
}

void Log::imprime_log()
{
    for (auto &&i : get_log())
    {
        std::cout   << " " << i.get()->id 
                    << " " << i.get()->id_user 
                    << " " << i.get()->tipo 
                    << " " << i.get()->ingreso 
                    << " " << i.get()->cambio 
                    << " " << i.get()->total 
                    << " " << i.get()->estatus
                    << " " << i.get()->fecha
                    << std::endl;
    }
     
}

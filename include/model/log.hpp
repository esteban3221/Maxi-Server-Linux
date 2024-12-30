#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <functional>

namespace Model
{
    struct Log_t
    {
        int id;
        int id_user;
        std::string tipo;
        int ingreso;
        int cambio;
        int total;
        std::string estatus;
        std::string fecha;

        Log_t() = default;
        explicit Log_t(const std::function<void(Log_t&)>& initializer) { initializer(*this); };
    };
} // namespace Model

class Log
{
private:
public:
    Log(/* args */);
    ~Log();

    std::vector<std::shared_ptr<Model::Log_t>> get_log();

    void imprime_log();
};

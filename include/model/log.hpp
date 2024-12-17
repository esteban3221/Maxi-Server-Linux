#pragma once
#include <iostream>
#include <vector>

struct Log_
{
    int id;
    int id_user;
    std::string tipo;
    int ingreso;
    int cambio;
    int total;
    std::string estatus;
    std::string fecha;
};

class Log
{
private:
public:
    Log(/* args */);
    ~Log();

    std::vector<Log_> get_log();
};

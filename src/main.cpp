
#include <iostream>
#include "log.hpp"

int main(int argc, char *argv[])
{
    Log lo;
    auto contenedor = lo.get_log();

    for (auto &&i : contenedor)
    {
        std::cout << "ID: " << i.id
        << " ID User: " << i.id_user
        << " Estatus: " << i.estatus
        << " Tipo: " << i.tipo
        << " Total: " << i.total
        << " Cambio: " << i.cambio
        << " Ingreso: " << i.ingreso
        << " Fecha: " << i.fecha << std::endl;
    }
    

    return 0;
}

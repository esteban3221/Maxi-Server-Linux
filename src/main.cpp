
#include <iostream>
#include "validator.hpp"

int main(int argc, char *argv[])
{
    Validator Coin("Coin");
    std::cout << Coin.Command_Post("Prueba").first << '\n';

    return 0;
}

#pragma once
#include <thread>

#include "view/config/validador.hpp"
#include "global.hpp"
#include "c_sharp_validator.hpp"

class DetallesValidador : public VDetallesValidador
{
private:
    Global::Async async_gui;
    void init_detalles();
public:
    DetallesValidador(/* args */);
    ~DetallesValidador();
};
#pragma once
#include <thread>

#include "view/config/validador.hpp"
#include "core/hub_cash.hpp"
#include "global.hpp"


class DetallesValidador : public VDetallesValidador
{
private:
    Global::Async async_gui;
    void init_validadores(void);
    // void conecta_validadores(const Global::EValidador::Conf &bill,const Global::EValidador::Conf &coin);

    // void on_btn_reconnectd();
public:
    DetallesValidador(/* args */);
    ~DetallesValidador();
};
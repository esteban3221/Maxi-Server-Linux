#pragma once
#include "session.hpp"
#include "global.hpp"
#include "view/main_window.hpp"

#include "controller/venta.hpp"
#include "controller/pago.hpp"
#include "controller/refill.hpp"

class MainWindow : public VMainWindow
{
private:
    Sesion sesion;



public:
    MainWindow(/* args */);
    ~MainWindow();
};

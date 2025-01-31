#pragma once
#include "session.hpp"
#include "global.hpp"
#include "c_sharp_validator.hpp"

#include "view/main_window.hpp"

#include "model/configuracion.hpp"

#include "controller/venta.hpp"
#include "controller/pago.hpp"
#include "controller/pago_manual.hpp"
#include "controller/refill.hpp"
#include "controller/config.hpp"

class MainWindow : public VMainWindow
{
private:
    Sesion sesion;
    sigc::connection conn;


public:
    MainWindow(/* args */);
    ~MainWindow();
};

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
#include "controller/nip.hpp"
#include "controller/nuevo_nip.hpp"

class MainWindow : public VMainWindow
{
private:
    Sesion sesion;
    sigc::connection conn;

    uint16_t contador_click;

    bool estado_validador();
    void on_btn_pill_clicked();

    void on_map_view();

public:
    MainWindow(/* args */);
    ~MainWindow();
};

#pragma once

#include "global.hpp"


#include "view/main_window.hpp"
#include "config/version.hpp"

#include "controller/session.hpp"
#include "controller/log.hpp"

#include "controller/venta.hpp"
#include "controller/pago.hpp"
#include "controller/pago_manual.hpp"
#include "controller/refill.hpp"
#include "controller/config.hpp"
#include "controller/configuracion.hpp"
#include "controller/nip.hpp"
#include "controller/nuevo_nip.hpp"
#include "controller/config/terminal.hpp"

class MainWindow : public VMainWindow
{
private:
    uint16_t contador_click;
    crow::SimpleApp app;
    std::future<void> future;

    Sesion sesion;
    LogData log_data;
    CConfiguracion configuracion;
    Terminal terminales;

    void on_btn_pill_clicked();

    void on_map_view();

public:
    MainWindow(/* args */);
    ~MainWindow();
};

#include "controller/main_window.hpp"

MainWindow::MainWindow(/* args */)
{
    // inicializacion de servidor
    Global::Rest::future = Global::Rest::app.port(44333).run_async();

    v_btn_pill->set_opacity(1);

    Glib::signal_timeout().connect([this]() -> bool 
    {
        if(Global::EValidador::is_retry_connected.load())
        {
            v_btn_pill->set_css_classes({"pill", "warning"});
            v_btn_pill->set_label("Detectando Validadores.");
        }
        else if (Global::EValidador::is_connected.load() &&
                 Global::EValidador::is_running.load())
        {
            v_btn_pill->set_css_classes({"pill", "suggested-action"});
            v_btn_pill->set_label("Validadores conectados y corriendo.");
        }
        else if (Global::EValidador::is_wrong_port.load())
        {
            v_btn_pill->set_css_classes({"pill", "destructive-action"});
            v_btn_pill->set_label("Uno o mas validadores no se iniciarion.\nPuerto incorrecto?");
        }
        else if (Global::EValidador::is_busy.load())
        {
            v_btn_pill->set_css_classes({"pill", "warning"});
            v_btn_pill->set_label("Ejecutando tarea.");
        }
        else if (not Global::EValidador::is_driver_correct.load())
        {
            v_btn_pill->set_css_classes({"pill", "error"});
            v_btn_pill->set_label("Driver no presente o en fallo.");
        }
        else
        {
            v_btn_pill->set_css_classes({"pill", "info"});
            v_btn_pill->set_label("Estado no previsto.");
        }

        return true;
    }, 1000);
}

MainWindow::~MainWindow()
{
    Global::Rest::app.stop();
    Global::Rest::future.get();
}
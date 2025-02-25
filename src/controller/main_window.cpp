#include "controller/main_window.hpp"
#include "main_window.hpp"

MainWindow::MainWindow(/* args */) : contador_click(0)
{
    // inicializacion de servidor
    Global::Rest::future = Global::Rest::app.port(44333).run_async();
    v_btn_pill->set_opacity(1);

    v_box_principal->signal_map().connect(sigc::mem_fun(*this, &MainWindow::on_map_view));

    conn = Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &MainWindow::estado_validador), 1);
    v_btn_logo_nip->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_btn_pill_clicked));
}

bool MainWindow::estado_validador()
{
    if (Global::EValidador::is_retry_connected.load())
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
}

void MainWindow::on_btn_pill_clicked()
{
    if (contador_click >= 4)
    {
        Global::Widget::v_main_stack->set_visible_child("6");
        contador_click = 0;
    }
    else
    {
        contador_click++;
        v_btn_pill->set_label(std::to_string(contador_click));
    }
}

void MainWindow::on_map_view()
{
    auto db_conf = std::make_unique<Configuracion>();
    auto data = db_conf->get_conf_data(6,7);

    v_img_main_logo->property_file() = data->get_item(0)->m_valor;
    v_lbl_main->set_text(data->get_item(1)->m_valor);
}

MainWindow::~MainWindow()
{
    std::system("killall dotnet");
    conn.disconnect();
    Global::Rest::app.stop();
    // Global::Rest::future.get();
}
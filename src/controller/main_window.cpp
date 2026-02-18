#include "controller/main_window.hpp"
#include "main_window.hpp"

MainWindow::MainWindow(/* args */) : contador_click(0)
{
    // inicializacion de servidor
    Global::Rest::future = Global::Rest::app.port(44333).loglevel(crow::LogLevel::Info).multithreaded().run_async();
    // por el momento es mejor que las peticiones sean en un solo hilo para evitar 
    // problemas de concurrencia con la base de datos y proceso con validadores
    v_btn_pill->set_opacity(1);

    v_box_principal->signal_map().connect(sigc::mem_fun(*this, &MainWindow::on_map_view));

    conn = Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &MainWindow::estado_validador), 1);
    v_btn_logo_nip->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_btn_pill_clicked));
}

bool MainWindow::estado_validador()
{
EstadoWS nuevo_estado;

    // Calcular el nuevo estado (igual que antes)
    if (Device::dv_bill.is_busy.load() || Device::dv_coin.is_busy.load())
    {
        nuevo_estado.css_classes = {"pill", "warning"};
        nuevo_estado.label = "Ejecutando tarea.";
        nuevo_estado.status = "Ejecutando tarea.";
        nuevo_estado.code = crow::status::CONTINUE;
    }
    else if (Global::EValidador::is_retry_connected.load())
    {
        nuevo_estado.css_classes = {"pill", "warning"};
        nuevo_estado.label = "Detectando Validadores.";
        nuevo_estado.status = "Detectando Validadores.";
        nuevo_estado.code = crow::status::TEMPORARY_REDIRECT;
    }
    else if (Global::EValidador::is_connected.load() &&
             Global::EValidador::is_running.load())
    {
        nuevo_estado.css_classes = {"pill", "suggested-action"};
        nuevo_estado.label = "Validadores conectados y corriendo.";
        nuevo_estado.status = "Validadores conectados y corriendo.";
        nuevo_estado.code = crow::status::OK;
    }
    else if (Global::EValidador::is_wrong_port.load())
    {
        nuevo_estado.css_classes = {"pill", "destructive-action"};
        nuevo_estado.label = "Uno o mas validadores no se iniciarion.\nPuerto incorrecto?";
        nuevo_estado.status = "Uno o mas validadores no se iniciarion. Puerto incorrecto?";
        nuevo_estado.code = crow::status::BAD_REQUEST;
    }
    else if (!Global::EValidador::is_driver_correct.load())
    {
        nuevo_estado.css_classes = {"pill", "error"};
        nuevo_estado.label = "Driver no presente o en fallo.";
        nuevo_estado.status = "Driver no presente o en fallo.";
        nuevo_estado.code = crow::status::INTERNAL_SERVER_ERROR;
    }
    else
    {
        nuevo_estado.css_classes = {"pill", "info"};
        nuevo_estado.label = "En Espera.";
        nuevo_estado.status = "En Espera.";
        nuevo_estado.code = crow::status::OK;
    }

    v_btn_pill->set_css_classes(nuevo_estado.css_classes);
    v_btn_pill->set_label(nuevo_estado.label);

    bool ha_cambiado = !primer_envio_realizado ||
                       nuevo_estado.code != ultimo_estado_enviado.code ||
                       nuevo_estado.status != ultimo_estado_enviado.status ||
                       nuevo_estado.label != ultimo_estado_enviado.label ||
                       nuevo_estado.css_classes != ultimo_estado_enviado.css_classes;

    if (ha_cambiado && Global::Rest::ws_connection)
    {
        auto json = crow::json::wvalue();
        json["status"] = nuevo_estado.status;
        json["code"]   = nuevo_estado.code;

        Global::Rest::ws_connection->send_text(json.dump());

        ultimo_estado_enviado = nuevo_estado;
        primer_envio_realizado = true;

        std::cout << "[WS] Estado enviado (cambio detectado): " << nuevo_estado.status << "\n";
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
    conn.disconnect();
    Global::Rest::app.stop();
    // Global::Rest::future.get();
}
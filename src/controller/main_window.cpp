#include "controller/main_window.hpp"

MainWindow::MainWindow(/* args */)
{
    // inicializacion de servidor
    Global::Rest::future = Global::Rest::app.port(44333).run_async();

    // trata de inciar coneccion a validadores con los datos guardados en BD
    auto db_conf = std::make_unique<Configuracion>();

    auto data_bill = db_conf->get_conf_data(1, 2);
    auto data_coin = db_conf->get_conf_data(3, 4);

    Global::EValidador::Conf conf_bill, conf_coin;
    conf_bill.puerto = data_bill->get_item(0)->m_valor;
    conf_bill.ssp = std::stoi(data_bill->get_item(1)->m_valor);

    conf_coin.puerto = data_coin->get_item(0)->m_valor;
    conf_coin.ssp = std::stoi(data_coin->get_item(1)->m_valor);

    conf_bill.auto_acepta_billetes = conf_coin.auto_acepta_billetes = false;
    conf_bill.habilita_recolector = conf_coin.habilita_recolector = false;
    conf_bill.dispositivo = conf_coin.dispositivo = "";
    conf_bill.log_ruta = conf_coin.log_ruta = "";

    Global::ApiConsume::autentica();

    //@@@esteban3221 hay que pasar los datos recolectados a sus respectivas vistas
    auto json_bill = Global::Device::dv_bill.inicia_dispositivo_v8(conf_bill);
    auto json_coin = Global::Device::dv_coin.inicia_dispositivo_v8(conf_coin);

    Global::ApiConsume::token.clear();

    if (json_bill["deviceModel"].s() != "UNKNOWN" && json_coin["deviceModel"].s() != "UNKNOWN")
    {
        v_btn_pill->set_opacity(1);
        v_btn_pill->set_css_classes({"pill", "suggested-action"});
        v_btn_pill->set_label("Validadores Conectados");
    }
    else
    {
        v_btn_pill->set_opacity(1);
        v_btn_pill->set_css_classes({"pill", "destructive-action"});
        v_btn_pill->set_label("Validadores Conectados");
    }
}

MainWindow::~MainWindow()
{
    Global::Rest::app.stop();
    Global::Rest::future.get();
}
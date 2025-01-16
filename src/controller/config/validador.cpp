#include "controller/config/validador.hpp"
#include "validador.hpp"

DetallesValidador::DetallesValidador(/* args */)
{
    std::system("killall dotnet");
    std::system("dotnet /opt/CashDevice_RestAPI_V1.3.1_.NET_8.0/CSharp_RestAPI.dll > /dev/null 2>&1 &");
    async_gui.dispatcher.connect(sigc::mem_fun(async_gui, &Global::Async::on_dispatcher_emit));
    v_btn_test_coneccion.signal_clicked().connect(sigc::mem_fun(*this, &DetallesValidador::on_btn_reconnectd));
    init_detalles();
}

DetallesValidador::~DetallesValidador()
{
}

void DetallesValidador::init_detalles()
{
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

    std::thread(&DetallesValidador::conecta_validadores, this, conf_bill, conf_coin).detach();
}

void DetallesValidador::conecta_validadores(const Global::EValidador::Conf &bill, const Global::EValidador::Conf &coin)
{
    using namespace Global::EValidador;

    try
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        Global::ApiConsume::autentica();
        is_retry_connected.store(true);

        async_gui.dispatch_to_gui([this]()
                                  { Global::Widget::v_main_stack->set_visible_child("0"); });

        auto json_bill = Global::Device::dv_bill.inicia_dispositivo_v8(bill);
        auto json_coin = Global::Device::dv_coin.inicia_dispositivo_v8(coin);

        crow::json::wvalue json_bill_copy(json_bill);
        crow::json::wvalue json_coin_copy(json_coin);

        Global::ApiConsume::token.clear();

        std::string device_bill = json_bill["deviceModel"].s();
        std::string device_coin = json_coin["deviceModel"].s();

        if (device_bill != "UNKNOWN" && device_coin != "UNKNOWN")
        {
            is_busy.store(false);
            is_connected.store(true);
            is_running.store(true);
            is_driver_correct.store(true);
            is_wrong_port.store(false);

            is_retry_connected.store(false);

            async_gui.dispatch_to_gui([this, json_bill_copy, json_coin_copy]()
                                      {
                    v_box_coin->set_data_lbl(json_coin_copy.dump());
                    v_box_bill->set_data_lbl(json_bill_copy.dump()); });
        }
        else
        {
            is_busy.store(false);
            is_connected.store(false);
            is_running.store(false);
            is_wrong_port.store(true);
            is_driver_correct.store(true);

            is_retry_connected.store(false);
        }
    }
    catch (const std::exception &e)
    {
        is_retry_connected.store(false);
        is_driver_correct.store(false);
        std::cerr << RED << "Error Fatal: " RESET << e.what() << '\n';
    }
}

void DetallesValidador::on_btn_reconnectd()
{
    std::system("killall dotnet");
    using namespace Global::EValidador;
    std::system("dotnet /opt/CashDevice_RestAPI_V1.3.1_.NET_8.0/CSharp_RestAPI.dll > /dev/null 2>&1 &");
    Global::EValidador::Conf conf_bill, conf_coin;
    conf_bill.puerto = v_box_bill->get_puerto_seleccionado();
    conf_bill.ssp = 0;

    conf_coin.puerto = v_box_coin->get_puerto_seleccionado();
    conf_coin.ssp = 16;

    conf_bill.auto_acepta_billetes = conf_coin.auto_acepta_billetes = false;
    conf_bill.habilita_recolector = conf_coin.habilita_recolector = false;
    conf_bill.dispositivo = conf_coin.dispositivo = "";
    conf_bill.log_ruta = conf_coin.log_ruta = "~/Documentos";

    auto db_conf = std::make_unique<Configuracion>();

    auto conf_bill_puerto = MConfiguracion::create(1, "Se actualizo el " + Glib::DateTime::create_now_local().format_iso8601(), conf_bill.puerto);
    auto conf_coin_puerto = MConfiguracion::create(3, "Se actualizo el " + Glib::DateTime::create_now_local().format_iso8601(), conf_coin.puerto);

    db_conf->update_conf(conf_coin_puerto);
    db_conf->update_conf(conf_bill_puerto);
    

    std::thread(&DetallesValidador::conecta_validadores, this, conf_bill, conf_coin).detach();
}

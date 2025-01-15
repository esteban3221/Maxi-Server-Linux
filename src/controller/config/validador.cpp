#include "controller/config/validador.hpp"
#include "validador.hpp"

DetallesValidador::DetallesValidador(/* args */)
{
    // dispatcher.connect(sigc::mem_fun(*this,&DetallesValidador::init_detalles));
    async_gui.dispatcher.connect(sigc::mem_fun(async_gui, &Global::Async::on_dispatcher_emit));

    init_detalles();
}

DetallesValidador::~DetallesValidador()
{
}

void DetallesValidador::init_detalles()
{
    using namespace Global::EValidador;

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

    std::thread([this, conf_bill, conf_coin]()
                {
        try 
        {
            is_retry_connected.store(true);

            auto json_bill = Global::Device::dv_bill.inicia_dispositivo_v8(conf_bill);
            auto json_coin = Global::Device::dv_coin.inicia_dispositivo_v8(conf_coin);

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

                async_gui.dispatch_to_gui([this,json_bill_copy,json_coin_copy]() {
                    v_box_coin->set_data_lbl(json_coin_copy.dump());
                    v_box_bill->set_data_lbl(json_bill_copy.dump());
                });
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
        } })
        .detach();
}

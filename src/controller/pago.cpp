#include "controller/pago.hpp"
#include "pago.hpp"

Pago::Pago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : BVentaPago(cobject, refBuilder)
{
    v_lbl_titulo->set_text("Pago");
    v_lbl_timeout->set_visible(false);
    v_BXRW4->set_visible(false);

    async_gui.dispatcher.connect(sigc::mem_fun(async_gui, &Global::Async::on_dispatcher_emit));

    CROW_ROUTE(RestApp::app, "/accion/inicia_pago").methods("POST"_method)(sigc::mem_fun(*this, &Pago::inicia));
}

Pago::~Pago()
{
}

void Pago::on_btn_retry_click()
{
    std::cout << "Click otra vez desde Pago\n";
}

void Pago::on_btn_cancel_click()
{
    std::cout << "Click cancela desde Pago\n";
}

bool Pago::pago_poll(int ant_coin, int ant_bill)
{
    int total_coin = 0, total_bill = 0;
    auto actual_level_coin = Device::dv_coin.get_level_cash_actual();
    auto actual_level_bill = Device::dv_bill.get_level_cash_actual();

    for (size_t i = 0; i < actual_level_coin->get_n_items(); i++)
    {
        auto m_list = actual_level_coin->get_item(i);
        total_coin += (m_list->m_denominacion / 100) * m_list->m_cant_recy;
    }

    for (size_t i = 0; i < actual_level_bill->get_n_items(); i++)
    {
        auto m_list = actual_level_bill->get_item(i);
        total_bill += (m_list->m_denominacion / 100) * m_list->m_cant_recy;
    }

    int32_t recibido =  (ant_coin + ant_bill) - (total_bill + total_coin);
    faltante = Global::EValidador::balance.total.load() - recibido;

    async_gui.dispatch_to_gui([this, recibido]()
    {
        v_lbl_faltante->set_text(std::to_string(faltante));
        v_lbl_recibido->set_text(std::to_string(recibido)); 
    });

    bool is_activo{faltante != 0};

    if (not is_activo)
    {
        Device::dv_coin.deten_cobro_v6();
        Device::dv_bill.deten_cobro_v6();
    }

    return is_activo;
}
crow::response Pago::inicia(const crow::request &req)
{
    using namespace Global::EValidador;
    auto bodyParams = crow::json::load(req.body);

    int cambio = bodyParams["value"].i();
    if (cambio <= 0)
        return crow::response("Nada que devolver");

    balance.total.store(bodyParams["value"].i());
    is_running.store(true);
    balance.ingreso.store(0);
    is_busy.store(true);

    if (conn.empty())
        conn.disconnect();

    std::cout << "\n ===== Obteniendo copia de estados de validadores ===== \n\n";

    s_level_mon = Device::map_cantidad_recyclador(Device::dv_coin);
    s_level_bill = Device::map_cantidad_recyclador(Device::dv_bill);

    async_gui.dispatch_to_gui([this, cambio]()
    { 
        auto s_total = std::to_string(cambio);
        Global::Widget::v_main_stack->set_visible_child(*this); 
        v_lbl_monto_total->set_text(s_total);
        v_lbl_faltante->set_text(s_total);
        v_lbl_recibido->set_text("0"); 
    });

    const auto total_ant_coin = Global::Utility::total_anterior(s_level_mon);
    const auto total_ant_bill = Global::Utility::total_anterior(s_level_bill);

    auto r_bill = Global::Utility::obten_cambio(cambio, s_level_bill);
    auto r_coin = Global::Utility::obten_cambio(cambio, s_level_mon);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    int status_bill, status_coin;
    const int max_intentos = 10; // Límite de reintentos

    // Procesamiento para dv_bill
    if (r_bill.dump() != "[0,0,0,0,0,0]") {
        Device::dv_bill.inicia_dispositivo_v6();
        int intentos_bill = 0;

        do {
            status_bill = Device::dv_bill.command_post("PayoutMultipleDenominations", r_bill.dump(), true).first;

            if (status_bill != crow::status::OK) {
                std::cout << "Estado de dv_bill no OK. Reintentando... (" << intentos_bill + 1 << "/" << max_intentos << ")" << std::endl;
                intentos_bill++;
                std::this_thread::sleep_for(std::chrono::seconds(1)); // Espera 1 segundo antes de reintentar
            }

            if (intentos_bill >= max_intentos) {
                std::cerr << "Número máximo de intentos alcanzado para dv_bill. Abortando..." << std::endl;
                break;
            }

        } while (status_bill != crow::status::OK);
    }

    // Procesamiento para dv_coin
    if (r_coin.dump() != "[0,0,0,0]") {
        Device::dv_coin.inicia_dispositivo_v6();
        int intentos_coin = 0;

        do {
            status_coin = Device::dv_coin.command_post("PayoutMultipleDenominations", r_coin.dump(), true).first;

            if (status_coin != crow::status::OK) {
                std::cout << "Estado de dv_coin no OK. Reintentando... (" << intentos_coin + 1 << "/" << max_intentos << ")" << std::endl;
                intentos_coin++;
                std::this_thread::sleep_for(std::chrono::seconds(1)); // Espera 1 segundo antes de reintentar
            }

            if (intentos_coin >= max_intentos) {
                std::cerr << "Número máximo de intentos alcanzado para dv_coin. Abortando..." << std::endl;
                break;
            }

        } while (status_coin != crow::status::OK);
    }

    start_time = std::chrono::steady_clock::now();
    conn = Glib::signal_timeout().connect(sigc::bind(sigc::mem_fun(*this, &Pago::pago_poll),total_ant_coin, total_ant_bill), 200);
    std::thread(&Global::Utility::verifica_cambio, &conn, start_time, [this]()
                { Global::System::showNotify("Pago", ("Falto dar cambio: " + std::to_string(faltante)).c_str(), "dialog-information"); })
        .detach();

    if (cambio > 0)
        Global::System::showNotify("Pago", ("Falto dar cambio: " + std::to_string(cambio)).c_str(), "dialog-information");

    crow::json::wvalue data_in =
        {
            {"Billetes", r_bill},
            {"Monedas", r_coin}
        };
    
    balance.ingreso.store(0);
    balance.cambio.store(0);

    return crow::response(data_in);
}

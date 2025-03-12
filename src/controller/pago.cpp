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

void Pago::da_pago(int cambio, const sigc::slot<bool ()> &slot, const std::string &tipo, std::string &estatus)
{
    auto s_level_mon = Device::map_cantidad_recyclador(Device::dv_coin);
    auto s_level_bill = Device::map_cantidad_recyclador(Device::dv_bill);

    const auto total_ant_coin = Global::Utility::total_anterior(s_level_mon);
    const auto total_ant_bill = Global::Utility::total_anterior(s_level_bill);

    auto r_bill = Global::Utility::obten_cambio(cambio, s_level_bill);
    auto r_coin = Global::Utility::obten_cambio(cambio, s_level_mon);

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

    auto start_time = std::chrono::steady_clock::now();
    auto conn = std::make_shared<sigc::connection>(Glib::signal_timeout().connect(slot, 200));
    
    Global::Utility::verifica_cambio(
        conn, 
        start_time, 
        [tipo, &estatus]()
        { 
            estatus = ("Falto dar cambio: " + std::to_string(faltante));
            Global::System::showNotify(tipo.c_str(), estatus.c_str(), "dialog-information"); 
        });

    if (cambio > 0)
    {
        estatus = ("Falto dar cambio: " + std::to_string(faltante));
        Global::System::showNotify(tipo.c_str(), estatus.c_str(), "dialog-information");
    }
        
}

crow::response Pago::inicia(const crow::request &req)
{
    using namespace Global::EValidador;
    auto bodyParams = crow::json::load(req.body);

    int cambio = balance.cambio = bodyParams["value"].i();
    if (cambio <= 0)
        return crow::response("Nada que devolver");

    balance.total.store(bodyParams["value"].i());
    is_running.store(true);
    balance.ingreso.store(0);
    is_busy.store(true);

    async_gui.dispatch_to_gui([this, cambio]()
    { 
        auto s_total = std::to_string(cambio);
        Global::Widget::v_main_stack->set_visible_child(*this); 
        v_lbl_monto_total->set_text(s_total);
        v_lbl_faltante->set_text(s_total);
        v_lbl_recibido->set_text("0"); 
    });

    std::cout << "\n ===== Obteniendo copia de estados de validadores ===== \n\n";
    
    auto s_level_mon = Device::map_cantidad_recyclador(Device::dv_coin);
    auto s_level_bill = Device::map_cantidad_recyclador(Device::dv_bill);

    const auto total_ant_coin = Global::Utility::total_anterior(s_level_mon);
    const auto total_ant_bill = Global::Utility::total_anterior(s_level_bill);

    const sigc::slot<bool ()> slot = sigc::bind(sigc::mem_fun(*this, &Pago::pago_poll),total_ant_coin, total_ant_bill);
    Pago::da_pago(balance.cambio.load(), slot, "Pago", estatus);

    Log log;
    auto t_log = MLog::create
    (
        0,
        Global::User::id,
        "Pago",
        0,
        balance.cambio.load(),
        0,
        Pago::faltante > 0 ? estatus : "Pago Realizada con Exito.",
        Glib::DateTime::create_now_local()
    );

    auto folio = log.insert_log(t_log);
    t_log->m_id = folio;

    if (Global::Widget::Impresora::is_activo)
    {
        std::string command = "echo \"" + Global::System::imprime_ticket(t_log, faltante) + "\" | lp";
        std::system(command.c_str());
    }

    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();
    
    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child("0"); });
    
    balance.ingreso.store(0);
    balance.cambio.store(0);

    return crow::response("Proceso Terminado");
}

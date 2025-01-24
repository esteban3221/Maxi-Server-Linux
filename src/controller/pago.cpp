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

void Pago::verifica_pago()
{
    while (conn.connected())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto now = std::chrono::steady_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();

        if (elapsed_seconds > 90)
        {
            std::cout << "El timeout sigue activo después de " << elapsed_seconds << " segundos." << std::endl;
            Global::System::showNotify("Pago", ("Falto dar" + std::to_string(faltante)).c_str(), "dialog-information");
            conn.disconnect();
        }
        else
            std::cout << "El timeout no está activo después de " << elapsed_seconds << " segundos. Todo Bien :3" << std::endl;
    }
}

std::map<int, int> Pago::cantidad_recyclador(const Validator &val)
{
    auto level = val.get_level_cash_actual();

    std::map<int, int> actual_level;

    for (size_t i = 0; i < level->get_n_items(); i++)
    {
        auto m_list = level->get_item(i);
        actual_level[m_list->m_denominacion] = m_list->m_cant_recy;
    }

    return actual_level;
}

void Pago::calcula_cambios(std::atomic_int32_t &salida_coin, std::atomic_int32_t &salida_bill, std::atomic_bool &terminado_coin, std::atomic_bool &terminado_bill)
{
    while (not terminado_bill.load() && not terminado_coin.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        async_gui.dispatch_to_gui([this, &salida_bill, &salida_coin]()
                                  {
            auto total_salida = salida_bill.load() + salida_coin.load();
            auto faltante = Global::EValidador::balance.total.load() - total_salida ;

            v_lbl_faltante->set_text(std::to_string(faltante));
            v_lbl_recibido->set_text(std::to_string(total_salida)); });
    }
    Global::EValidador::is_running.store(false);
    Global::Device::dv_coin.deten_cobro_v6();
    Global::Device::dv_bill.deten_cobro_v6();
}

bool Pago::pago_poll()
{
    int total_coin = 0, total_bill = 0;
    auto actual_level_coin = Global::Device::dv_coin.get_level_cash_actual();
    auto actual_level_bill = Global::Device::dv_bill.get_level_cash_actual();

    for (size_t i = 0; i < actual_level_coin->get_n_items(); i++)
    {
        auto m_list = actual_level_coin->get_item(i);
        total_coin += m_list->m_cant_recy - s_level_mon[m_list->m_denominacion];
    }

    for (size_t i = 0; i < actual_level_bill->get_n_items(); i++)
    {
        auto m_list = actual_level_bill->get_item(i);
        total_bill += m_list->m_cant_recy - s_level_bill[m_list->m_denominacion];
    }

    faltante = total_bill + total_coin;
    int32_t total = Global::EValidador::balance.total.load() - faltante;

    async_gui.dispatch_to_gui([this, total]()
                              {
        v_lbl_faltante->set_text(std::to_string(faltante));
        v_lbl_recibido->set_text(std::to_string(total)); });

    bool is_activo{faltante != 0};

    if (not is_activo)
    {
        Global::Device::dv_coin.deten_cobro_v6();
        Global::Device::dv_bill.deten_cobro_v6();
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

    s_level_mon = cantidad_recyclador(Global::Device::dv_coin);
    s_level_bill = cantidad_recyclador(Global::Device::dv_bill);

    async_gui.dispatch_to_gui([this, cambio]()
                              { 
        auto s_total = std::to_string(cambio);
        Global::Widget::v_main_stack->set_visible_child(*this); 
        v_lbl_monto_total->set_text(s_total);
        v_lbl_faltante->set_text(s_total);
        v_lbl_recibido->set_text("0"); });

    auto r_bill = Global::Utility::obten_cambio(cambio, s_level_bill);
    auto r_coin = Global::Utility::obten_cambio(cambio, s_level_mon);

    if (r_bill.dump() != "[0,0,0,0,0,0]")
    {
        Global::Device::dv_bill.inicia_dispositivo_v6();
        Global::Device::dv_bill.command_post("PayoutMultipleDenominations", r_bill.dump(), true);
    }

    if (r_coin.dump() != "[0,0,0,0]")
    {
        Global::Device::dv_coin.inicia_dispositivo_v6();
        Global::Device::dv_coin.command_post("PayoutMultipleDenominations", r_coin.dump(), true);
    }

    start_time = std::chrono::steady_clock::now();

    std::thread(&Pago::verifica_pago, this).detach();

    conn = Glib::signal_timeout().connect(sigc::mem_fun(*this, &Pago::pago_poll), 300);

    if (cambio > 0)
        Global::System::showNotify("Pago", ("Falto dar cambio: " + std::to_string(cambio)).c_str(), "dialog-information");

    crow::json::wvalue data_in =
        {
            {"Billetes", r_bill},
            {"Monedas", r_coin}};

    return crow::response(data_in);
}

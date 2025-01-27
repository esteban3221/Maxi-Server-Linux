#include "controller/venta.hpp"
#include "venta.hpp"

Venta::Venta(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : BVentaPago(cobject, refBuilder)
{
    v_lbl_titulo->set_text("Venta");
    v_btn_timeout_retry->set_visible(false);

    async_gui.dispatcher.connect(sigc::mem_fun(async_gui, &Global::Async::on_dispatcher_emit));

    CROW_ROUTE(RestApp::app, "/accion/inicia_venta").methods("POST"_method)(sigc::mem_fun(*this, &Venta::inicia));
}

Venta::~Venta()
{
}

void Venta::on_btn_retry_click()
{
    std::cout << "Click otra vez desde Venta\n";
}

void Venta::on_btn_cancel_click()
{
    deten(crow::request());
}

bool Venta::pago_poll()
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

    bool is_activo{faltante != 0};

    if (not is_activo)
    {
        Global::Device::dv_coin.deten_cobro_v6();
        Global::Device::dv_bill.deten_cobro_v6();
    }

    return is_activo;
}

void Venta::verifica_pago()
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

void Venta::da_cambio()
{
    s_level_mon = cantidad_recyclador(Global::Device::dv_coin);
    s_level_bill = cantidad_recyclador(Global::Device::dv_bill);

    int cambio = Global::EValidador::balance.cambio.load();

    if (conn.empty())
        conn.disconnect();

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
    std::thread(&Venta::verifica_pago, this).detach();
    conn = Glib::signal_timeout().connect(sigc::mem_fun(*this, &Venta::pago_poll), 300);
    
    if (cambio > 0)
        Global::System::showNotify("Pago", ("Falto dar cambio: " + std::to_string(cambio)).c_str(), "dialog-information");
    
}

std::map<int, int> Venta::cantidad_recyclador(const Validator &val)
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

void Venta::func_poll(const std::string &status, const crow::json::rvalue &data)
{
    using namespace Global::EValidador;
    // Monedero
    if (status == "COIN_CREDIT" ||
        status == "VALUE_ADDED" ||
        status == "ESCROW")
    {
        std::cout << "Se recibio: " << data["value"].i() << '\n';
        balance.ingreso_parcial.store(data["value"].i());
        Global::Device::dv_bill.acepta_dinero(status, true);

        balance.ingreso.store(balance.ingreso.load() + (data["value"].i() / 100));
        async_gui.dispatch_to_gui([this]()
                                  {
            v_lbl_recibido->set_text(std::to_string(balance.ingreso.load()));

            auto faltante = balance.ingreso.load() > balance.total.load() ? 0 : balance.total.load() - balance.ingreso.load();
            balance.cambio.store(balance.ingreso.load() > balance.total.load() ? balance.ingreso.load() - balance.total.load() : 0);

            v_lbl_cambio->set_text(std::to_string(balance.cambio.load()));
            v_lbl_faltante->set_text(std::to_string(faltante)); });

        if (balance.ingreso.load() >= balance.total.load())
        {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            on_btn_cancel_click();
        }
    }
}

crow::response Venta::inicia(const crow::request &req)
{
    using namespace Global::EValidador;
    auto bodyParams = crow::json::load(req.body);
    balance.total.store(bodyParams["value"].i());
    balance.ingreso.store(0);
    balance.cambio.store(0);

    is_busy.store(true);
    is_running.store(true);

    async_gui.dispatch_to_gui([this, bodyParams]()
                              { 
        auto s_total = std::to_string(bodyParams["value"].i());
        Global::Widget::v_main_stack->set_visible_child(*this); 
        v_lbl_monto_total->set_text(s_total);
        v_lbl_faltante->set_text(s_total);
        v_lbl_cambio->set_text("0");
        v_lbl_recibido->set_text("0"); });

    Global::Device::dv_coin.inicia_dispositivo_v6();
    Global::Device::dv_bill.inicia_dispositivo_v6();

    auto future1 = std::async(std::launch::async, [this]() { Global::Device::dv_coin.poll(sigc::mem_fun(*this, &Venta::func_poll)); });
    auto future2 = std::async(std::launch::async, [this]() { Global::Device::dv_bill.poll(sigc::mem_fun(*this, &Venta::func_poll)); });

    future1.wait();
    future2.wait();

    if (balance.cambio.load() > 0)
        da_cambio();

    is_busy.store(false);
    /// @@@ agregar lo del cambio
    return crow::response();
}

crow::response Venta::deten(const crow::request &req)
{
    Global::EValidador::is_running.store(false);
    Global::Device::dv_coin.deten_cobro_v6();
    Global::Device::dv_bill.deten_cobro_v6();
    return crow::response();
}

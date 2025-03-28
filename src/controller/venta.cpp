#include "controller/venta.hpp"
#include "venta.hpp"

Venta::Venta(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : BVentaPago(cobject, refBuilder),
                                                                                      faltante(0)
{
    v_lbl_titulo->set_text("Venta");
    v_btn_timeout_retry->set_visible(false);

    async_gui.dispatcher.connect(sigc::mem_fun(async_gui, &Global::Async::on_dispatcher_emit));

    CROW_ROUTE(RestApp::app, "/accion/inicia_venta").methods("POST"_method)(sigc::mem_fun(*this, &Venta::inicia));
    CROW_ROUTE(RestApp::app, "/accion/deten_venta").methods("POST"_method)(sigc::mem_fun(*this, &Venta::deten));
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

bool Venta::pago_poll(int ant_coin, int ant_bill)
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

    int32_t recibido = (ant_coin + ant_bill) - (total_bill + total_coin);
    Pago::faltante = Global::EValidador::balance.cambio.load() - recibido;

    bool is_activo{Pago::faltante != 0};

    if (not is_activo)
    {
        Device::dv_coin.deten_cobro_v6();
        Device::dv_bill.deten_cobro_v6();
    }

    return is_activo;
}

void Venta::func_poll(const std::string &status, const crow::json::rvalue &data)
{
    using namespace Global::EValidador;
    // Monedero
    if (status == "COIN_CREDIT" ||
        status == "VALUE_ADDED" ||
        status == "ESCROW")
    {
        balance.ingreso_parcial.store(data["value"].i());
        Device::dv_bill.acepta_dinero(status, true);

        balance.ingreso.store(balance.ingreso.load() + (data["value"].i() / 100));
        async_gui.dispatch_to_gui([this]()
                                  {
            v_lbl_recibido->set_text(std::to_string(balance.ingreso.load()));

            faltante = balance.ingreso.load() > balance.total.load() ? 0 : balance.total.load() - balance.ingreso.load();
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
    estatus.clear();
    faltante = bodyParams["value"].i();
    balance.total.store(faltante);
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
        v_lbl_recibido->set_text("0"); 
    });

    Device::dv_coin.inicia_dispositivo_v6();
    Device::dv_bill.inicia_dispositivo_v6();

    auto future1 = std::async(std::launch::async, [this](){ Device::dv_coin.poll(sigc::mem_fun(*this, &Venta::func_poll)); });
    auto future2 = std::async(std::launch::async, [this](){ Device::dv_bill.poll(sigc::mem_fun(*this, &Venta::func_poll)); });

    future1.wait();
    future2.wait();

    if (balance.cambio.load() > 0)
    {
        auto s_level_mon = Device::map_cantidad_recyclador(Device::dv_coin);
        auto s_level_bill = Device::map_cantidad_recyclador(Device::dv_bill);

        const auto total_ant_coin = Global::Utility::total_anterior(s_level_mon);
        const auto total_ant_bill = Global::Utility::total_anterior(s_level_bill);

        const sigc::slot<bool()> slot = sigc::bind(sigc::mem_fun(*this, &Venta::pago_poll), total_ant_coin, total_ant_bill);
        Pago::da_pago(balance.cambio.load(), slot, "Venta", estatus);
    }

    crow::json::wvalue data;
    Log log;
    auto t_log = MLog::create
    (
        0,
        Global::User::id,
        "Venta",
        balance.ingreso.load(),
        balance.cambio.load(),
        balance.total.load(),
        Pago::faltante > 0 ? estatus : "Venta Realizada con Exito.",
        Glib::DateTime::create_now_local()
    );

    auto folio = log.insert_log(t_log);
    t_log->m_id = folio;

    if (Global::Widget::Impresora::v_switch_impresion->get_active())
    {
        std::string command = "echo \"" + Global::System::imprime_ticket(t_log, faltante) + "\" | lp";
        std::system(command.c_str());
    }

    data["total"] = balance.total.load();
    data["ingreso"] = balance.ingreso.load();
    data["faltante"] = faltante;
    data["cambio"] = balance.cambio.load();
    data["Cambio_faltante"] = Pago::faltante;

    is_busy.store(false);

    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();

    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child("0"); });

    return crow::response(data);
}

crow::response Venta::deten(const crow::request &req)
{
    Global::EValidador::is_running.store(false);
    estatus = "Venta cancelada";
    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();
    return crow::response();
}

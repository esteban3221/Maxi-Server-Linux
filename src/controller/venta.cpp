#include "controller/venta.hpp"

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

void Venta::func_poll(const std::string &status, const crow::json::rvalue &data)
{
    using namespace Global::EValidador;

    if (status == "STACKED")
    {
        auto s_level = Device::dv_bill.get_level_cash_actual(true);

        for (size_t i = 0; i < s_level->get_n_items(); i++)
        {
            auto m_list = s_level->get_item(i);
            if (m_list->m_denominacion == balance.ingreso_parcial / 100)
            {
                auto m_list_ant = s_level_ant->get_item(i);

                if (m_list_ant->m_cant_recy < m_list->m_cant_recy)
                {
                    auto db = std::make_unique<LevelCash>("Level_Bill");
                    m_list_ant->m_cant_alm++;
                    db->update_level_cash(m_list_ant);
                }
                break;
            }
        }
    }

    if (status == "COIN_CREDIT" ||
        status == "VALUE_ADDED" ||
        status == "ESCROW")
    {
        balance.ingreso_parcial.store(data["value"].i());
        Device::dv_bill.acepta_dinero(status, true);
        s_level_ant = Device::dv_coin.get_level_cash_actual(true);

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
            std::this_thread::sleep_for(std::chrono::seconds(1));
            Global::EValidador::is_running.store(false);
            Device::dv_coin.deten_cobro_v6();
            Device::dv_bill.deten_cobro_v6();
        }
    }
}

crow::response Venta::inicia(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Venta);

    using namespace Global::EValidador;
    auto bodyParams = crow::json::load(req.body);
    cancelado = false;
    estatus.clear();
    faltante = bodyParams["value"].i();
    std::string concepto;
    concepto = bodyParams.has("concepto") ? bodyParams["concepto"].operator std::string() : "*- Sin Concepto -*";
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

    if (cancelado)
    {
        estatus = "Venta cancelada";
        if (balance.ingreso.load() > 0)
            Pago::da_pago(balance.ingreso.load(), "Venta", estatus);
    }   
    else if (balance.cambio.load() > 0)
        Pago::da_pago(balance.cambio.load(), "Venta", estatus);
    else if (Pago::faltante > 0)
        estatus = "Venta incompleta, faltante: " + std::to_string(Pago::faltante);
    
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
        "- " + concepto + " | " + (not Global::Utility::is_ok || cancelado ? estatus : "Venta Realizada con Exito."),
        Glib::DateTime::create_now_local()
    );

    t_log->m_id = log.insert_log(t_log);
    t_log->m_estatus = concepto + '\n' + (not Global::Utility::is_ok || cancelado ? estatus : "Venta Realizada con Exito.");
    Global::Utility::is_ok = true;

    if (Global::Widget::Impresora::v_switch_impresion->get_active())
    {
        std::string command = "echo \"" + Global::System::imprime_ticket(t_log, faltante) + "\" | lp";
        std::system(command.c_str());
    }

    data = Global::Utility::json_ticket(t_log);
    data["Cambio_faltante"] = Pago::faltante;
    Global::EValidador::is_busy.store(false);

    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();

    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child("0"); });

    return crow::response(data);
}

crow::response Venta::deten(const crow::request &req)
{
    Global::EValidador::is_running.store(false);
    cancelado = true;
    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();
    return crow::response();
}

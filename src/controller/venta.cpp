#include "controller/venta.hpp"

Venta::Venta(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : BVentaPago(cobject, refBuilder),
                                                                                      faltante(0)
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
    Global::EValidador::is_running.store(false);
    cancelado = true;
}

void Venta::func_poll(const std::string &status, const crow::json::rvalue &data)
{
    using namespace Global::EValidador;

    if (status == "ESCROW")
    {
        auto bill_selection = Device::dv_bill.get_level_cash_actual(true);
        for (size_t i = 0; i < bill_selection->get_n_items(); i++)
        {
            auto m_list = bill_selection->get_typed_object<MLevelCash>(i);
            if (m_list->m_denominacion == (data["value"].i() / 100))
                if (m_list->m_cant_recy <= m_list->m_nivel_inmo_min)
                    Device::dv_bill.acepta_dinero(m_list->m_denominacion, true);
                else if (m_list->m_cant_recy >= m_list->m_nivel_inmo_max)
                    Device::dv_bill.acepta_dinero(m_list->m_denominacion, false);
        }
    }

    if (status == "COIN_CREDIT" ||
        status == "VALUE_ADDED" ||
        status == "ESCROW" )
    {
        s_level_ant = Device::dv_coin.get_level_cash_actual(true);

        balance.ingreso.store(balance.ingreso.load() + (data["value"].i() / 100));
        async_gui.dispatch_to_gui([this]()
        {
            v_lbl_recibido->set_text(std::to_string(balance.ingreso.load()));

            faltante = balance.ingreso.load() > balance.total.load() ? 0 : balance.total.load() - balance.ingreso.load();
            balance.cambio.store(balance.ingreso.load() > balance.total.load() ? balance.ingreso.load() - balance.total.load() : 0);

            v_lbl_cambio->set_text(std::to_string(balance.cambio.load()));
            v_lbl_faltante->set_text(std::to_string(faltante)); 
        });

        if (balance.ingreso.load() >= balance.total.load())
        {
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

    bool is_view_ingreso = bodyParams.has("is_view_ingreso") && bodyParams["is_view_ingreso"].b();
    is_view_ingreso ? v_lbl_titulo->set_text("Ingreso") : v_lbl_titulo->set_text("Venta");

    std::string concepto = bodyParams.has("concepto") && bodyParams["concepto"].s().size() > 0 ? 
    bodyParams["concepto"].operator std::string() : 
    "*- Sin Concepto -*";
    balance.total.store(faltante);
    balance.ingreso.store(0);
    balance.cambio.store(0);
    Pago::faltante = 0;

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
        estatus = "Operación cancelada";
        if (balance.ingreso.load() > 0)
            Pago::da_pago(balance.ingreso.load(), is_view_ingreso ? "Ingreso" : "Venta", estatus);
    }   
    else if (balance.cambio.load() > 0)
        Pago::da_pago(balance.cambio.load(), is_view_ingreso ? "Ingreso" : "Venta", estatus);
    if (Pago::faltante > 0)
        estatus = "Cambio Incompleto, faltante: " + std::to_string(Pago::faltante);
    
    crow::json::wvalue data;
    Log log;
    auto t_log = MLog::create
    (
        0,
        Global::User::id,
        is_view_ingreso ? "Ingreso" : "Venta",
        balance.ingreso.load(),
        balance.cambio.load(),
        balance.total.load(),
        "- " + concepto + " | " + (not Global::Utility::is_ok || cancelado ? estatus : "Exito."),
        Glib::DateTime::create_now_local()
    );

    t_log->m_id = log.insert_log(t_log);
    t_log->m_estatus = concepto + '\n' + (not Global::Utility::is_ok || cancelado ? estatus : "Exito.");
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

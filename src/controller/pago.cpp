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

void Pago::pago_poll(const std::string &state, const crow::json::rvalue &json)
{
    auto dum = crow::json::wvalue(json);
    std::cout << dum.dump() << "\n-\n";

    for (auto &&i : json)
    {
        if (i.has("value"))
        {
            Global::EValidador::balance.cambio.store(json["value"].i() / 100);

            async_gui.dispatch_to_gui([this]()
                                      {
                    auto total_salida = Global::EValidador::balance.cambio.load();
                    auto faltante = Global::EValidador::balance.total.load() - total_salida ;

                    v_lbl_faltante->set_text(std::to_string(faltante));
                    v_lbl_recibido->set_text(std::to_string(total_salida)); });

            Global::EValidador::is_running.store(false);
        }
        else if (i.has("data"))
        {
            if(Global::EValidador::balance.cambio.load() >= Global::EValidador::balance.total.load())
                Global::EValidador::is_running.store(false);
        }
    }

    // else if (state == "ERROR" && json.has("data"))
    // {
    //     async_gui.dispatch_to_gui([this]()
    //     {
    //         v_lbl_mensaje_fin->set_visible();
    //         v_lbl_mensaje_fin->set_text("Error al iniciar Pago");
    //     });
    //     terminado.store(true);
    // }
}

crow::response Pago::inicia(const crow::request &req)
{
    using namespace Global::EValidador;
    auto bodyParams = crow::json::load(req.body);
    balance.total.store(bodyParams["value"].i());
    Global::EValidador::is_running.store(true);

    int cambio = bodyParams["value"].i();

    auto s_level_mon = cantidad_recyclador(Global::Device::dv_coin);
    auto s_level_bill = cantidad_recyclador(Global::Device::dv_bill);

    async_gui.dispatch_to_gui([this, cambio]()
                              { 
        auto s_total = std::to_string(cambio);
        Global::Widget::v_main_stack->set_visible_child(*this); 
        v_lbl_monto_total->set_text(s_total);
        v_lbl_faltante->set_text(s_total);
        v_lbl_recibido->set_text("0"); });

    auto r_bill = Global::Utility::obten_cambio(cambio, s_level_bill);
    auto r_coin = Global::Utility::obten_cambio(cambio, s_level_mon);

    std::atomic_bool terminado_coin{false}, terminado_bill{false};
    std::atomic_int32_t salida_coin{0}, salida_bill{0}, total{0};
    std::future<void> future1, future2, future3;

    if (r_bill.dump() != "[0,0,0,0,0,0]")
    {
        Global::Device::dv_bill.inicia_dispositivo_v6();
        Global::Device::dv_bill.command_post("PayoutMultipleDenominations", r_bill.dump(), true);

        //auto future2 = std::async(std::launch::async, [this](){ Global::Device::dv_bill.poll(sigc::mem_fun(*this, &Pago::pago_poll)); });
    }

    if (r_coin.dump() != "[0,0,0,0]")
    {
        Global::Device::dv_coin.inicia_dispositivo_v6();
        Global::Device::dv_coin.command_post("PayoutMultipleDenominations", r_coin.dump(), true);

        //auto future1 = std::async(std::launch::async, [this](){ Global::Device::dv_coin.poll(sigc::mem_fun(*this, &Pago::pago_poll)); });
    }

    if (future1.valid())
        future1.wait();
    if (future2.valid())
        future2.wait();

    crow::json::wvalue data_in =
        {
            {"Billetes", r_bill},
            {"Monedas", r_coin}};

    return crow::response(data_in);
}

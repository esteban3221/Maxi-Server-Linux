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

crow::response Pago::inicia(const crow::request &req)
{   
    using namespace Global::EValidador;
    auto bodyParams = crow::json::load(req.body);
    balance.total.store(bodyParams["value"].i());

    int cambio = bodyParams["value"].i();

    auto s_level_mon = cantidad_recyclador(Global::Device::dv_coin);
    auto s_level_bill = cantidad_recyclador(Global::Device::dv_bill);

    auto r_bill = Global::Utility::obten_cambio(cambio, s_level_bill);
    auto r_coin = Global::Utility::obten_cambio(cambio, s_level_mon);


    Global::Device::dv_coin.inicia_dispositivo_v6();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    Global::Device::dv_bill.inicia_dispositivo_v6();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    Global::Device::dv_bill.command_post("PayoutMultipleDenominations",r_bill.dump(), true);
    Global::Device::dv_coin.command_post("PayoutMultipleDenominations",r_coin.dump(), true);

    crow::json::wvalue data_in =
        {
            {"Billetes", r_bill},
            {"Monedas", r_coin}
        };
    

    // async_gui.dispatch_to_gui([this, bodyParams]() 
    // { 
    //     auto s_total = std::to_string(bodyParams["value"].i());
    //     Global::Widget::v_main_stack->set_visible_child(*this); 
    //     v_lbl_monto_total->set_text(s_total);
    //     v_lbl_faltante->set_text(s_total);
    //     v_lbl_recibido->set_text("0");
    // });



    return crow::response(data_in);
}

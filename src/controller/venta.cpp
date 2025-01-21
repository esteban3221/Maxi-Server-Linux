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
    std::cout << "Click cancela desde Venta\n";
    deten(crow::request());
}

void Venta::func_poll(const std::string &status, const crow::json::rvalue &data)
{
    using namespace Global::EValidador;
    //Monedero
    if (status == "COIN_CREDIT" ||
        status == "VALUE_ADDED" ||
        status == "ESCROW")
    {
        std::cout << "Se recibio: " << data["value"].i() << '\n';
        balance.ingreso_parcial.store(data["value"].i());
        Global::Device::dv_bill.acepta_dinero(status,true);

        balance.ingreso.store(balance.ingreso.load() + (data["value"].i() / 100));
        async_gui.dispatch_to_gui([this]()
        {
            v_lbl_recibido->set_text(std::to_string(balance.ingreso.load()));
            auto faltante = balance.ingreso.load() > balance.total.load() ? 0 : balance.total.load() - balance.ingreso.load();
            v_lbl_faltante->set_text(std::to_string(faltante));
        });

        if(balance.ingreso.load() >= balance.total.load())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
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
    
    Global::Device::dv_coin.inicia_dispositivo_v6();
    Global::Device::dv_bill.inicia_dispositivo_v6();
    
    auto future1 = std::async(std::launch::async, [this]() { Global::Device::dv_coin.poll(sigc::mem_fun(*this, &Venta::func_poll)); });
    auto future2 = std::async(std::launch::async, [this]() { Global::Device::dv_bill.poll(sigc::mem_fun(*this, &Venta::func_poll)); });

    future1.wait();
    future2.wait();

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

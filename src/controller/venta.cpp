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
    if (status == "COIN_CREDIT" ||
        status == "VALUE_ADDED" ||
        status == "ESCROW")
    {
        std::cout << "Se recibio: " << data["value"].i() << '\n';
    }
}

crow::response Venta::inicia(const crow::request &req)
{
    async_gui.dispatch_to_gui([this]() { Global::Widget::v_main_stack->set_visible_child(*this); });
    Global::Device::dv_coin.inicia_dispositivo_v6();
    Global::Device::dv_coin.poll(sigc::mem_fun(*this, &Venta::func_poll));

    return crow::response();
}

crow::response Venta::deten(const crow::request &req)
{
    Global::EValidador::is_running.store(false);
    Global::Device::dv_coin.deten_cobro_v6();
    return crow::response();
}

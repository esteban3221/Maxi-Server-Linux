#include "controller/pago.hpp"
#include "pago.hpp"

Pago::Pago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : BVentaPago(cobject, refBuilder)
{
    v_lbl_titulo->set_text("Pago");
    v_lbl_timeout->set_visible(false);
    v_BXRW4->set_visible(false);

    async_gui.dispatcher.connect(sigc::mem_fun(async_gui, &Global::Async::on_dispatcher_emit));

    CROW_ROUTE(RestApp::app, "/accion/inicia_pago").methods("POST"_method)(sigc::mem_fun(*this, &Pago::inicia));
    CROW_ROUTE(RestApp::app, "/accion/inicia_pago_manual").methods("POST"_method)(sigc::mem_fun(*this, &Pago::inicia_manual));
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

void Pago::poll_pago(const std::pair<int, std::string> &status)
{
    auto json_data = crow::json::load(status.second);
    if (json_data.has("dispenseResult"))
        if (json_data["dispenseResult"].s() == "COMPLETED")
        {
            Global::System::showNotify("Pago", "Pago completado con éxito", "dialog-information");
        }
        else if (json_data["dispenseResult"].s() == "IN_PROGRESS")
        {
            /*continue*/
        }
        else if (json_data["dispenseResult"].s() == "ERROR")
        {
            Global::System::showNotify("Pago", "Error al procesar el pago: Error en validador", "dialog-error");
        }
        else if (json_data["dispenseResult"].s() == "TIME_OUT")
        {
            Global::System::showNotify("Pago", "Pago Incompleto", "dialog-warning");
        }
}

void Pago::da_pago(int cambio, const std::string &tipo, std::string &estatus)
{
    Pago::faltante = 0;
    auto s_level_mon = Device::map_cantidad_recyclador(Device::dv_coin);
    auto s_level_bill = Device::map_cantidad_recyclador(Device::dv_bill);

    auto r_bill = Global::Utility::obten_cambio(cambio, s_level_bill);
    auto r_coin = Global::Utility::obten_cambio(cambio, s_level_mon);

    if (cambio > 0)
        Pago::faltante = cambio;

    da_pago(r_bill.dump(), r_coin.dump(), tipo, estatus);
}

void Pago::da_pago(const std::string &bill, const std::string &coin, const std::string &tipo, std::string &estatus)
{
    std::pair<int, std::string> status_bill = {200,""}, status_coin = {200,""};
    int max_intentos = 0;

    if (bill == "[0,0,0,0,0,0]" && coin == "[0,0,0,0]")
    {
        Global::Utility::is_ok = false;
        Global::System::showNotify(tipo.c_str(), "No se cuenta con suficiente efectivo", "dialog-error");
        Pago::faltante = Global::EValidador::balance.cambio.load();
    }
    else
    {
        // Procesamiento para dv_bill
        if (bill != "[0,0,0,0,0,0]")
        {
            Device::dv_bill.inicia_dispositivo_v6();

            status_bill = Device::dv_bill.reintenta_comando_post("PayoutMultipleDenominations", bill, max_intentos);
            poll_pago(status_bill);
        }

        // Procesamiento para dv_coin
        if (coin != "[0,0,0,0]")
        {
            Device::dv_coin.inicia_dispositivo_v6();

            status_coin = Device::dv_coin.reintenta_comando_post("PayoutMultipleDenominations", coin , max_intentos);
            poll_pago(status_coin);
        }

        if (status_bill.first != crow::status::OK || status_coin.first != crow::status::OK)
        {
            estatus = "Error al procesar el pago";
            Global::System::showNotify(tipo.c_str(), estatus.c_str(), "dialog-error");
        }
    }
    
}

crow::response Pago::inicia(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Cambio_A);
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

    Pago::da_pago(balance.cambio.load(), "Pago", estatus);

    Log log;
    crow::json::wvalue data;
    auto t_log = MLog::create(
        0,
        Global::User::id,
        "Pago",
        0,
        balance.cambio.load(),
        0,
        not Global::Utility::is_ok ? estatus : "Pago Realizada con Exito.",
        Glib::DateTime::create_now_local());

    t_log->m_id = log.insert_log(t_log);
    Global::Utility::is_ok = true;

    if (Global::Widget::Impresora::v_switch_impresion->get_active())
    {
        std::string command = "echo \"" + Global::System::imprime_ticket(t_log, faltante) + "\" | lp";
        std::system(command.c_str());
    }

    data = Global::Utility::json_ticket(t_log);
    data["Cambio_faltante"] = Pago::faltante;

    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();

    async_gui.dispatch_to_gui([this]() { Global::Widget::v_main_stack->set_visible_child("0"); });

    balance.ingreso.store(0);
    balance.cambio.store(0);

    return crow::response(data);
}

crow::response Pago::inicia_manual(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Cambio_M);
    using namespace Global::EValidador;
    auto bodyParams = crow::json::load(req.body);

    int cambio = balance.cambio = bodyParams["total"].i();
    balance.total = cambio;

    std::vector<int> bill_values;
    for (auto &&i : bodyParams["bill"])
        bill_values.push_back(i.i());
    

    std::vector<int> coin_values;
    for (auto &&i : bodyParams["coin"])
        coin_values.push_back(i.i());


    auto s_level_mon = Device::map_cantidad_recyclador(Device::dv_coin);
    auto s_level_bill = Device::map_cantidad_recyclador(Device::dv_bill);

    int total = 0;

    for (size_t i = 0; i < s_level_mon.size(); i++)
    {
        if (coin_values[i] > s_level_mon.at(map_coin.at(i)))
            return crow::response(crow::status::CONFLICT, "No hay suficiente cambio en la denominacion: " + std::to_string(map_coin.at(i)) + " de monedas");
        total += coin_values[i] * map_coin.at(i);
    }

    for (size_t i = 0; i < s_level_bill.size(); i++)
    {
        if (bill_values[i] > s_level_bill.at(map_bill.at(i)))
            return crow::response(crow::status::CONFLICT, "No hay suficiente cambio en la denominacion: " + std::to_string(map_bill.at(i)) + " de billetes");
        total += bill_values[i] * map_bill.at(i);
    }

    is_running.store(true);
    balance.ingreso.store(0);
    is_busy.store(true);

    async_gui.dispatch_to_gui([this, total]()
    { 
        Global::Widget::v_main_stack->set_visible_child(*this); 
        v_lbl_monto_total->set_text("Monto Manual: " + std::to_string(total));
        v_lbl_faltante->set_text("0");
        v_lbl_recibido->set_text("0"); 
    });

    std::string bill = "[";
    for (size_t i = 0; i < bill_values.size(); i++)
        bill += std::to_string(bill_values[i]) + ",";
    bill.pop_back();
    bill += "]";
    std::string coin = "[";
    for (size_t i = 0; i < coin_values.size(); i++)
        coin += std::to_string(coin_values[i]) + ",";
    coin.pop_back();
    coin += "]";

    const auto total_ant_coin = Global::Utility::total_anterior(s_level_mon);
    const auto total_ant_bill = Global::Utility::total_anterior(s_level_bill);

    Pago::da_pago(bill, coin, "Pago Manual", estatus);

    Log log;
    crow::json::wvalue data;
    auto t_log = MLog::create
    (
        0,
        Global::User::id,
        "Pago Manual",
        0,
        total,
        0,
        not Global::Utility::is_ok ? estatus : "Pago Realizada con Exito.",
        Glib::DateTime::create_now_local());
    Global::Utility::is_ok = true;
    t_log->m_id = log.insert_log(t_log);

    if (Global::Widget::Impresora::v_switch_impresion->get_active())
    {
        std::string command = "echo \"" + Global::System::imprime_ticket(t_log, faltante) + "\" | lp";
        std::system(command.c_str());
    }

    data = Global::Utility::json_ticket(t_log);
    data["Cambio_faltante"] = Pago::faltante;

    Device::dv_coin.deten_cobro_v6();
    Device::dv_bill.deten_cobro_v6();

    async_gui.dispatch_to_gui([this](){ Global::Widget::v_main_stack->set_visible_child("0"); });

    return crow::response(data);
}

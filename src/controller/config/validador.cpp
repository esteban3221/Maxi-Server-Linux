#include "controller/config/validador.hpp"

DetallesValidador::DetallesValidador(/* args */)
{
    crow::logger::setLogLevel(crow::LogLevel::Debug);
    async_gui.dispatcher.connect(sigc::mem_fun(async_gui, &Global::Async::on_dispatcher_emit));
    v_btn_test_coneccion.signal_clicked().connect(sigc::mem_fun(*this, &DetallesValidador::init_validadores));
    init_validadores();
}

DetallesValidador::~DetallesValidador()
{
}

void DetallesValidador::init_validadores(void)
{
    auto &hub = CashHub::instance();
    hub.inicializar_hardware();

    // Solo para pruebas
    
    Conf conf;
    conf.habilita_recolector = true;
    conf.auto_acepta_credito = true;
    conf.habilita_salida_credito = true;

    hub.inicia_for_all(conf,{});
    
    hub.command_for_all(HttpMethod::POST, "SmartEmpty", R"({
"ModuleNumber": 0,
"IsNV4000": false
})", true);

    hub.detiene_for_all();
}

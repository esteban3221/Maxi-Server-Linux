#include "controller/config/validador.hpp"

DetallesValidador::DetallesValidador(/* args */)
{
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

    ///@todo: Mostrar datos de información de los validadores en la GUI
}

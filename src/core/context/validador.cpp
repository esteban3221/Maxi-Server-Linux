#include "core/context/validador.hpp"
#include "core/states/svalidador.hpp"

ValidadorUnit::ValidadorUnit(/* args */) : handler_state(std::make_unique<EstadoIdle>()),
                                           nombre_validator("Desconocido"),
                                           ingreso_credito(0),
                                           salida_credito(0),
                                           poll_milli(200),
                                           poll(false)
{
    handler_state->on_entry(*this);
}

ValidadorUnit::~ValidadorUnit()
{
}

// context/validador.cpp (implementación)
void ValidadorUnit::transiciona_estado(std::unique_ptr<IValidador> nuevo_estado)
{
    if (!nuevo_estado)
        return;

    if (handler_state)
    {
        handler_state->on_exit(*this);
    }

    auto estado_anterior = handler_state ? handler_state->get_nombre_estado() : "Ninguno";
    handler_state = std::move(nuevo_estado);

    handler_state->on_entry(*this);
    signal_state_changed.emit(handler_state->get_nombre_estado());

    CROW_LOG_INFO << "Transición: " << estado_anterior << " -> " << handler_state->get_nombre_estado() << '\n';
}

void ValidadorUnit::imprime_debug(const cpr::Response &r) const
{
    CROW_LOG_DEBUG << "Validador:" << nombre_validator << "\n"
                   << "URL: " << r.url.str() << '\n'
                   << "Tiempo: " << r.elapsed << '\n'
                   << "Codigo: " << r.status_code << '\n'
                   << "Respuesta: " << r.text << '\n';
}

const cpr::Response ValidadorUnit::command_get(const std::string &command, bool debug) const
{
    auto response = cpr::Get(cpr::Url{BASE_URL, "/", command},
                             cpr::Header{{"Content-Type", "application/json"}});

    if (debug)
        imprime_debug(response);

    return response;
}
const cpr::Response ValidadorUnit::command_post(const std::string &command, const std::string &json, bool debug) const
{
    auto response = cpr::Post(cpr::Url{BASE_URL, "/", command},
                              cpr::Header{{"Content-Type", "application/json"}},
                              cpr::Body{json});

    if (debug)
        imprime_debug(response);

    return response;
}
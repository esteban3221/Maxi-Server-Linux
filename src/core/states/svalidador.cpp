#include "context/validador.hpp"
#include "states/svalidador.hpp"

// --- Implementación IDLE ---
void EstadoIdle::on_entry(ValidadorUnit &v) { CROW_LOG_INFO << "Entrando a Idle" << v.property_device_model(); }
void EstadoIdle::on_start_init(ValidadorUnit &v, const Conf &c, const crow::json::rvalue &r) {
    v.transiciona_estado(std::make_unique<EstadoIniciando>());
}
void EstadoIdle::on_handle_event(ValidadorUnit &v, const std::string &e, const crow::json::rvalue &d) {}

// --- Implementación INICIANDO ---
void EstadoIniciando::on_entry(ValidadorUnit &v) { CROW_LOG_INFO << "Iniciando hardware..."; }
void EstadoIniciando::on_start_init(ValidadorUnit &v, const Conf &conf, const crow::json::rvalue &set_routes) {
    v.inicia_conecta(set_routes);
    // Si inicia_conecta no falló (no mandó a Error), pasamos a Activo
    if (v.get_nombre_estado() == "Iniciando") {
        v.transiciona_estado(std::make_unique<EstadoActivo>());
    }
}
void EstadoIniciando::on_handle_event(ValidadorUnit &v, const std::string &e, const crow::json::rvalue &d) {}

// --- Implementación ACTIVO ---
void EstadoActivo::on_entry(ValidadorUnit &v) { CROW_LOG_INFO << "Validador Activo"; }
void EstadoActivo::on_stop_disconnect(ValidadorUnit &v) {
    v.transiciona_estado(std::make_unique<EstadoDeteniendo>());
}
void EstadoActivo::on_handle_event(ValidadorUnit &v, const std::string &event_type, const crow::json::rvalue &data) {
    // El estado Activo sabe que estos eventos significan "Dinero entrante"
    if (event_type == "ESCROW" || event_type == "STACKED" || event_type == "VALUE_ADDED") {
        CROW_LOG_INFO << "Crédito detectado en Estado Activo: " << event_type;
        v.signal_event_received.emit(event_type, data);
    } 
    else if (event_type == "FRAUD_ATTEMPT") {
        v.signal_error.emit("Posible intento de fraude detectado");
    }
    else if (event_type == "JAMMED") {
        v.signal_error.emit("Dispositivo atascado");
        // transicionar a un estado de Error automáticamente
        v.transiciona_estado(std::make_unique<EstadoError>("Atasco detectado"));
    }
    else if (event_type == "INCOMPLETE_PAYOUT" || event_type == "ERROR_DURING_PAYOUT") {
        v.signal_error.emit("Pago incompleto detectado");
        // transicionar a un estado de Error automáticamente
        v.transiciona_estado(std::make_unique<EstadoError>("Error genérico detectado: " + data.operator std::filesystem::__cxx11::path::string_type()));
    }
    else if (event_type == "ERROR")
    {
        v.signal_error.emit("Error genérico detectado: " + data.operator std::filesystem::__cxx11::path::string_type());
        // transicionar a un estado de Error automáticamente
        v.transiciona_estado(std::make_unique<EstadoError>("Error genérico detectado: " + data.operator std::filesystem::__cxx11::path::string_type()));
    }
}

// --- Implementación DETENIENDO ---
void EstadoDeteniendo::on_entry(ValidadorUnit &v) {
    //v.detiene_desconecta();
    v.transiciona_estado(std::make_unique<EstadoIdle>());
}
void EstadoDeteniendo::on_handle_event(ValidadorUnit &v, const std::string &e, const crow::json::rvalue &d) {}

// --- Implementación ERROR ---
void EstadoError::on_entry(ValidadorUnit &v) {
    CROW_LOG_ERROR << "Estado Error: " << mensaje;
    v.signal_error.emit(mensaje);
}
void EstadoError::on_handle_event(ValidadorUnit &v, const std::string &e, const crow::json::rvalue &d) {
    if (e == "RESET") v.transiciona_estado(std::make_unique<EstadoIdle>());
}
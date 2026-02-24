//"interfaces/ivalidador.hpp"
#pragma once
#include "context/validador.hpp"
#include <utility>
#include <iostream>
#include <crow.h>
/*
Idle → Iniciando → Conectando → Activo → Pausado → Deteniendo → Idle
  ↑                                                    |
  └────────────────────────────────────────────────────┘
*/
class ValidadorUnit;

class IValidador {
public:
    virtual ~IValidador() = default;
    
    // Métodos esenciales del patrón State
    virtual void on_entry(ValidadorUnit& v) {}      // Al entrar al estado
    virtual void on_exit(ValidadorUnit& v) {}       // Al salir del estado
    
    // Métodos específicos del validador
    virtual void on_start_init(ValidadorUnit& v) {}
    virtual void on_stop_disconnect(ValidadorUnit& v) {}
    virtual void on_handle_event(ValidadorUnit& v, const std::string& event_type, const crow::json::rvalue& data) = 0;
    
    // Getters
    virtual std::string get_nombre_estado() const = 0;
    virtual bool puede_iniciar() const { return false; }
    virtual bool puede_detener() const { return false; }
};
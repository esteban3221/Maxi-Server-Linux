#pragma once
#include <crow.h>
#include <thread>
#include <glibmm.h>

#include "usuarios.hpp"
#include "usuarios_roles.hpp"
#include "global.hpp"

#include "config/version.hpp"
#include "shared/version_comparator.hpp"
#include "core/hub_cash.hpp"

class Sesion //: public sigc::trackable
{
private:
    crow::response login(const crow::request &req);
    crow::response alta_usuario(const crow::request &req);
    crow::response baja_usuario(const crow::request &req);
    crow::response modifica_usuario(const crow::request &req);
    crow::response modifica_usuario_roles(const crow::request &req);
    crow::response logout(const crow::request &req);

    crow::response get_all_users(const crow::request &req);
    crow::response get_all_roles_by_id(const crow::request &req);
    crow::response maxicajero_version_check(const crow::request &req);

    friend class CashHub;
    static std::string token;

    // WebSocket
    void on_websocket_open(crow::websocket::connection& conn);
    void on_websocket_close(crow::websocket::connection& conn, const std::string& reason, uint16_t code);
    void on_websocket_message(crow::websocket::connection& conn, const std::string& data, bool is_binary);

public:
    Sesion(crow::SimpleApp& app);
    ~Sesion();

    static void valida_autorizacion(const crow::request &req, Global::User::Rol rol);
    static bool valida_administrador(const crow::request &req);
};
#pragma once
#include <crow.h>
#include <thread>
#include <glibmm.h>

#include "usuarios.hpp"
#include "usuarios_roles.hpp"
#include "global.hpp"
#include "c_sharp_validator.hpp"
#include "config/version.hpp"
#include "shared/version_comparator.hpp"

namespace RestApp = Global::Rest;
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

    // WebSocket
    void on_websocket_open(crow::websocket::connection& conn);
    void on_websocket_close(crow::websocket::connection& conn, const std::string& reason, uint16_t code);
    void on_websocket_message(crow::websocket::connection& conn, const std::string& data, bool is_binary);

public:
    Sesion(/* args */);
    ~Sesion();
};
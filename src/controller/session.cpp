#include "session.hpp"

std::string Sesion::token = "";
Sesion::Sesion(/* args */)
{

    // dispatcher.connect(sigc::mem_fun(*this, &session_controller::on_dispatcher_emit));
    //
    // callbacks de peticiones

    CROW_ROUTE(RestApp::app, "/sesion/login").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::login));
    CROW_ROUTE(RestApp::app, "/sesion/alta_usuario").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::alta_usuario));
    CROW_ROUTE(RestApp::app, "/sesion/baja_usuario").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::baja_usuario));
    CROW_ROUTE(RestApp::app, "/sesion/modifica_usuario").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::modifica_usuario));
    CROW_ROUTE(RestApp::app, "/sesion/modifica_usuario_roles").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::modifica_usuario_roles));
    CROW_ROUTE(RestApp::app, "/sesion/logout").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::logout));

    CROW_ROUTE(RestApp::app, "/sesion/get_all_users").methods("GET"_method)(sigc::mem_fun(*this, &Sesion::get_all_users));
    CROW_ROUTE(RestApp::app, "/sesion/get_all_roles_by_id").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::get_all_roles_by_id));
    CROW_ROUTE(RestApp::app, "/api/compatibilidad").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::maxicajero_version_check));

    // WebSocket route
    CROW_WEBSOCKET_ROUTE(RestApp::app, "/ws/heartbeat")
        .onopen(sigc::mem_fun(*this, &Sesion::on_websocket_open))
        .onclose(sigc::mem_fun(*this, &Sesion::on_websocket_close))
        .onmessage(sigc::mem_fun(*this, &Sesion::on_websocket_message));
}

Sesion::~Sesion()
{
}

crow::response Sesion::login(const crow::request &req)
{
    if (req.body.empty())
        return crow::response(crow::status::BAD_REQUEST);

    auto bodyParams = req.get_body_params();
    std::string grantType{bodyParams.pop("grant_type")};
    std::string password{bodyParams.pop("password")};

    auto usuarios = std::make_unique<Usuarios>();

    if (auto username = usuarios->existe_usuario(password);
        not username.second.empty())
    {
        crow::json::wvalue json;
        Global::User::id = username.first;

        json["usuario"] = Global::User::Current = username.second;
        json["token"] = token = CashHub::instance().autentica();

        return crow::response(json);
    }

    return crow::response(crow::status::UNAUTHORIZED);
}

crow::response Sesion::logout(const crow::request &req)
{
    token.clear();
    return crow::response();
}

bool Sesion::valida_administrador(const crow::request &req)
{
    auto auth_header = req.get_header_value("Authorization");
    if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ")
        throw BadRequestError("Error en la peticion, no se recibieron datos / token");
    std::string mycreds = auth_header.substr(7);
    if (mycreds != token)
        throw UnauthorizedError("token invalido");
    size_t cont_admin = 0;
    UsuariosRoles u_roles;
    auto roles = u_roles.get_usuario_roles_by_id(Global::User::id);
    return roles->get_n_items() >= 19;
}

void Sesion::valida_autorizacion(const crow::request &req, Global::User::Rol rol)
{
    auto auth_header = req.get_header_value("Authorization");
    if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ")
        throw BadRequestError("Error en la peticion, no se recibieron datos / token");

    std::string mycreds = auth_header.substr(7);

    if (mycreds != token)
        throw UnauthorizedError("token invalido");

    UsuariosRoles u_roles;
    auto roles = u_roles.get_usuario_roles_by_id(Global::User::id);

    if (roles)
    {
        for (size_t i = 0; i < roles->get_n_items(); i++)
        {
            auto list = roles->get_item(i);
            if (list->m_id_rol == static_cast<guint>(rol))
                return;
        }
        throw UnauthorizedError("No autorizado para realizar esta operacion");
    }
    else
        throw ServerError("Error al cargar roles");
}

void Sesion::on_websocket_open(crow::websocket::connection &conn)
{
    CROW_LOG_INFO << "New WebSocket connection established";
}

void Sesion::on_websocket_close(crow::websocket::connection &conn, const std::string &reason, uint16_t code)
{
    CROW_LOG_INFO << "WebSocket connection closed: " << reason;
}

void Sesion::on_websocket_message(crow::websocket::connection &conn, const std::string &data, bool is_binary)
{
    CROW_LOG_INFO << "Received WebSocket message: ";
}

crow::response Sesion::get_all_users(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Configuracion);
    auto usuarios = std::make_unique<Usuarios>();
    auto m_list = usuarios->get_usuarios();
    crow::json::wvalue json;

    for (size_t i = 0; i < m_list->get_n_items(); i++)
    {
        auto usuario = m_list->get_item(i);
        json["usuarios"][i]["id"] = usuario->m_id;
        json["usuarios"][i]["username"] = usuario->m_usuario;
        json["usuarios"][i]["password"] = "********";
    }

    return crow::response(json);
}

crow::response Sesion::get_all_roles_by_id(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Configuracion);
    auto bodyParams = crow::json::load(req.body);
    auto id_usuario = bodyParams["id_usuario"].i();

    UsuariosRoles u_roles;
    auto roles = u_roles.get_usuario_roles_by_id(id_usuario);
    if (roles)
    {
        crow::json::wvalue json;

        for (size_t i = 0; i < roles->get_n_items(); i++)
        {
            auto list = roles->get_item(i);
            json["roles"][i]["id"] = list->m_id;
            json["roles"][i]["id_usuario"] = list->m_id_usuario;
            json["roles"][i]["id_rol"] = list->m_id_rol;
        }
        return crow::response(json);
    }
    return crow::response(crow::status::NOT_FOUND);
}

crow::response Sesion::alta_usuario(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Configuracion);
    auto bodyParams = crow::json::load(req.body);

    auto nuevo_usuario = MUsuarios::create((size_t)1, // de momento no se sabe que id tiene
                                           Glib::ustring{bodyParams["username"].s()},
                                           Glib::ustring{bodyParams["password"].s()});

    auto usuarios = std::make_unique<Usuarios>();
    nuevo_usuario->m_id = usuarios->insert_usuario(nuevo_usuario);

    return crow::response();
}

crow::response Sesion::baja_usuario(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Configuracion);

    auto bodyParams = crow::json::load(req.body);
    auto id_usuario = bodyParams["id"].i();
    auto usuarios = std::make_unique<Usuarios>();
    auto usuario = usuarios->get_usuarios(id_usuario);

    if (usuario)
    {
        usuarios->delete_usuario(usuario);
    }
    else
    {
        return crow::response(crow::status::NOT_FOUND);
    }

    return crow::response();
}

crow::response Sesion::modifica_usuario(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Configuracion);

    auto bodyParams = crow::json::load(req.body);

    auto usuario = MUsuarios::create(
        bodyParams["id"].i(),
        Glib::ustring{bodyParams["username"].s()},
        Glib::ustring{bodyParams["password"].s()});

    auto usuarios = std::make_unique<Usuarios>();
    usuarios->update_usuario(usuario);
    return crow::response();
}

crow::response Sesion::modifica_usuario_roles(const crow::request &req)
{
    Sesion::valida_autorizacion(req, Global::User::Rol::Configuracion);

    auto bodyParams = crow::json::load(req.body);
    auto id_usuario = bodyParams["id_usuario"].i();

    auto roles = std::make_unique<UsuariosRoles>();
    auto list_roles = Gio::ListStore<MRoles>::create();
    if (bodyParams["roles"].t() != crow::json::type::Null)
        for (size_t i = 0; i < bodyParams["roles"].size(); i++)
        {
            list_roles->append(MRoles::create(bodyParams["roles"][i].i(), ""));
        }
    roles->update_usuario_roles(id_usuario, list_roles);

    return crow::response();
}

crow::response Sesion::maxicajero_version_check(const crow::request &req)
{
    auto json_data = crow::json::load(req.body);
    std::string client_version = json_data["version"].s();
    std::string plataform = json_data["plataform"].s();

    CROW_LOG_INFO << "Client version: " << client_version;
    CROW_LOG_INFO << "Client platform: " << plataform;
    CROW_LOG_INFO << "IP Client: " << req.remote_ip_address;

    Maxicajero::VersionUtils::CompatibilityChecker checker;
    Maxicajero::VersionUtils::Version clientVer = Maxicajero::VersionUtils::Version::fromString(client_version);
    Maxicajero::VersionUtils::Version serverVer = Maxicajero::VersionUtils::Version::fromString(Maxicajero::Version::getVersion());

    crow::json::wvalue response;
    crow::status status_code;
    ;

    if (!checker.isCompatible(clientVer, serverVer, Maxicajero::VersionUtils::CompatibilityChecker::Policy::FORWARD))
    {
        response["status"] = "incompatible";
        response["message"] = checker.getCompatibilityMessage(clientVer, serverVer, Maxicajero::VersionUtils::CompatibilityChecker::Policy::FORWARD);
        status_code = crow::status::NOT_ACCEPTABLE;
    }
    else
    {
        CROW_LOG_INFO << "Versiones compatible: " << client_version << " <= " << Maxicajero::Version::getVersion();
        response["status"] = "compatible";
        response["local_server_version"] = Maxicajero::Version::getVersion();
        status_code = crow::status::OK;
    }

    return crow::response(status_code, response);
}
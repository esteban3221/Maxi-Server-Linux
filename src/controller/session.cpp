#include "session.hpp"

Sesion::Sesion(/* args */)
{

    // dispatcher.connect(sigc::mem_fun(*this, &session_controller::on_dispatcher_emit));
    //
    // callbacks de peticiones

    CROW_ROUTE(RestApp::app, "/sesion/login").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::login));
    CROW_ROUTE(RestApp::app, "/sesion/alta_usuario").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::alta_usuario));
    CROW_ROUTE(RestApp::app, "/sesion/logout").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::logout));
    CROW_ROUTE(RestApp::app, "/test_coneccion").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::poll_status));

    CROW_ROUTE(RestApp::app, "/sesion/get_all_users").methods("GET"_method)(sigc::mem_fun(*this, &Sesion::get_all_users));
    CROW_ROUTE(RestApp::app, "/sesion/get_all_roles_by_id").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::get_all_roles_by_id));
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
        Global::ApiConsume::autentica();

        crow::json::wvalue json;
        Global::User::id = username.first;

        json["usuario"] = Global::User::Current = username.second;
        json["token"] = Global::ApiConsume::token;

        return crow::response(json);
    }

    return crow::response(crow::status::UNAUTHORIZED);
}

crow::response Sesion::logout(const crow::request &req)
{
    Global::ApiConsume::token.clear();
    return crow::response();
}

crow::response Sesion::poll_status(const crow::request &req)
{
    return crow::response(crow::status::OK);
}

crow::response Sesion::get_all_users(const crow::request &req)
{
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
    auto bodyParams = crow::json::load(req.body);
    auto id_usuario = bodyParams["id_usuario"].i();

    UsuariosRoles u_roles;
    auto roles = u_roles.get_usuario_roles_by_id(id_usuario);

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

crow::response Sesion::alta_usuario(const crow::request &req)
{
    if (req.body.empty())
        return crow::response(crow::status::BAD_REQUEST);

    //@@@esteban3221 falta validar permisos
    auto bodyParams = crow::json::load(req.body);

    auto nuevo_usuario = MUsuarios::create((size_t)1, // de momento no se sabe que id tiene
                                           Glib::ustring{bodyParams["username"].s()},
                                           Glib::ustring{bodyParams["password"].s()});

    auto usuarios = std::make_unique<Usuarios>();
    nuevo_usuario->m_id = usuarios->insert_usuario(nuevo_usuario);

    //@@@esteban3221 falta agregar cambios a la vista

    return crow::response();
}

crow::response Sesion::baja_usuario(const crow::request &req)
{
    return crow::response();
}

crow::response Sesion::modifica_usuario(const crow::request &req)
{
    return crow::response();
}

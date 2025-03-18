#include "session.hpp"

Sesion::Sesion(/* args */)
{

    // dispatcher.connect(sigc::mem_fun(*this, &session_controller::on_dispatcher_emit));
    //
    // callbacks de peticiones

    CROW_ROUTE(RestApp::app, "/sesion/login").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::login));
    CROW_ROUTE(RestApp::app, "/sesion/alta_usuario").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::alta_usuario));
    CROW_ROUTE(RestApp::app, "/test_coneccion").methods("POST"_method)(sigc::mem_fun(*this, &Sesion::poll_status));
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

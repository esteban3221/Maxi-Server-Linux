#include "controller/config/terminal.hpp"

Terminal::Terminal(/* args */)
{
    CROW_ROUTE(RestApp::app, "/terminales").methods("GET"_method)(sigc::mem_fun(*this, &Terminal::get_all));
    CROW_ROUTE(RestApp::app, "/terminales/<string>").methods("GET"_method)(sigc::mem_fun(*this, &Terminal::get_by_id));
    CROW_ROUTE(RestApp::app, "/terminales/nueva").methods("POST"_method)(sigc::mem_fun(*this, &Terminal::nueva));
    CROW_ROUTE(RestApp::app, "/terminales/editar").methods("POST"_method)(sigc::mem_fun(*this, &Terminal::editar));
    CROW_ROUTE(RestApp::app, "/terminales/eliminar").methods("DELETE"_method)(sigc::mem_fun(*this, &Terminal::eliminar));
}

Terminal::~Terminal()
{
}

crow::response Terminal::get_all(const crow::request &req)
{
    auto terminales = std::make_unique<OTerminales>();
    auto contenedor_terminales = terminales->get_all();

    crow::json::wvalue json;
    json["terminales"] = crow::json::wvalue::list();

    for (size_t i = 0; i < contenedor_terminales->at("id").size(); i++)
    {
        json["terminales"][i]["id"] = contenedor_terminales->at("id")[i];
        json["terminales"][i]["tipo"] = contenedor_terminales->at("tipo")[i];
        json["terminales"][i]["alias"] = contenedor_terminales->at("alias")[i];
        json["terminales"][i]["modo"] = contenedor_terminales->at("modo")[i];
        json["terminales"][i]["predeterminado"] = std::stoi(contenedor_terminales->at("predeterminado")[i]) == 1;
        json["terminales"][i]["descripcion"] = contenedor_terminales->at("descripcion")[i];
        json["terminales"][i]["fecha_creado"] = contenedor_terminales->at("fecha_creado")[i];
    }

    return crow::response(json);
}

crow::response Terminal::get_by_id(const std::string &id)
{
    auto terminales = std::make_unique<OTerminales>();
    auto terminal = terminales->get_by_id(id);

    if (!terminal)
        return crow::response(404);

    crow::json::wvalue json;
    json["id"] = terminal->m_id;
    json["tipo"] = terminal->m_tipo;
    json["alias"] = terminal->m_alias;
    json["modo"] = terminal->m_modo;
    json["predeterminado"] = terminal->m_predeterminado;
    json["descripcion"] = terminal->m_descripcion;
    json["fecha_creado"] = terminal->m_fecha_creado.format_iso8601();

    return crow::response(json);
}

crow::response Terminal::nueva(const crow::request &req)
{
    auto bodyParams = crow::json::load(req.body);

    std::string id = bodyParams["id"].s();
    std::string tipo = bodyParams["tipo"].s();
    std::string alias = bodyParams["alias"].s();
    std::string modo = bodyParams["modo"].s();
    std::string access_token = bodyParams["access_token"].s();
    // auto predeterminado = bodyParams["predeterminado"].b();
    std::string descripcion = bodyParams["descripcion"].s();

    auto terminal = MTerminales::create(id, tipo, alias, modo, access_token, false, descripcion, Glib::DateTime::create_now_local());
    auto terminales = std::make_unique<OTerminales>();
    terminales->inserta(terminal);

    return crow::response(crow::status::CREATED);
}

crow::response Terminal::editar(const crow::request &req)
{
    auto bodyParams = crow::json::load(req.body);

    std::string id = bodyParams["id"].s();
    auto terminales = std::make_unique<OTerminales>();
    auto terminal = terminales->get_by_id(id);
    if (!terminal)
        return crow::response(crow::status::CONFLICT);

    terminal->m_tipo = bodyParams.has("tipo") ? (Glib::ustring)bodyParams["tipo"].s() : terminal->m_tipo;
    terminal->m_alias = bodyParams.has("alias") ? (Glib::ustring)bodyParams["alias"].s() : terminal->m_alias;
    terminal->m_modo = bodyParams.has("modo") ? (Glib::ustring)bodyParams["modo"].s() : terminal->m_modo;
    terminal->m_predeterminado = bodyParams.has("predeterminado") ? bodyParams["predeterminado"].b() : terminal->m_predeterminado;
    terminal->m_descripcion = bodyParams.has("descripcion") ? (Glib::ustring)bodyParams["descripcion"].s() : terminal->m_descripcion;

    terminales->edita(terminal);
    if (terminal->m_predeterminado)
        terminales->predetermina(terminal->m_id);

    return crow::response(200);
}

crow::response Terminal::eliminar(const crow::request &req)
{
    auto bodyParams = crow::json::load(req.body);
    std::string id = bodyParams["id"].s();

    auto terminales = std::make_unique<OTerminales>();
    terminales->elimina(id);

    return crow::response(200);
}
#include "controller/log.hpp"

LogData::LogData(/* args */)
{
    CROW_ROUTE(RestApp::app, "/log/movimientos").methods("POST"_method)(sigc::mem_fun(*this, &LogData::get_log));
    CROW_ROUTE(RestApp::app, "/log/corte_caja").methods("GET"_method)(sigc::mem_fun(*this, &LogData::corte_caja));
}

LogData::~LogData()
{
}

crow::response LogData::corte_caja(const crow::request &req)
{
    auto log = std::make_unique<Log>();
    auto bodyParams = crow::json::load(req.body);
    auto tipo = bodyParams["tipo"].s();
    crow::json::wvalue json;
    json["log"] = crow::json::wvalue::list();

    auto contenedor_log = Global::Utility::valida_administrador(req)
                              ? log->get_corte(0, tipo)
                              : log->get_corte(Global::User::id, tipo);

    for (size_t i = 0; i < contenedor_log->at("Id").size(); i++)
    {
        json["log"][i]["id"] = std::stoull(contenedor_log->at("Id")[i]);
        json["log"][i]["id_user"] = std::stoull(contenedor_log->at("IdUser")[i]);
        json["log"][i]["tipo"] = contenedor_log->at("Tipo")[i];
        json["log"][i]["ingreso"] = std::stoi(contenedor_log->at("Ingreso")[i]);
        json["log"][i]["cambio"] = std::stoi(contenedor_log->at("Cambio")[i]);
        json["log"][i]["total"] = std::stoi(contenedor_log->at("Total")[i]);
        json["log"][i]["estatus"] = contenedor_log->at("Estatus")[i];
        json["log"][i]["fecha"] = contenedor_log->at("Fecha")[i];
    }

    return crow::response(json);
}

crow::response LogData::get_log(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Consulta_Movimientos);
    auto bodyParams = crow::json::load(req.body);

    auto tipo = bodyParams["tipo"].s();
    auto f_ini = bodyParams["f_ini"].s();
    auto f_fin = bodyParams["f_fin"].s();
    auto pag = bodyParams["pag"].i();

    auto log = std::make_unique<Log>();
    auto user = std::make_unique<Usuarios>();

    auto contenedor_log = log->get_log(tipo, f_ini, f_fin, pag);

    crow::json::wvalue json;

    json["total_rows"] = log->tam_row;
    json["log"] = crow::json::wvalue::list();
    for (size_t i = 0; i < contenedor_log->get_n_items(); i++)
    {
        json["log"][i]["id"] = contenedor_log->get_item(i)->m_id;
        json["log"][i]["usuario"] = user->get_usuarios(contenedor_log->get_item(i)->m_id_user)->m_usuario;
        json["log"][i]["tipo"] = contenedor_log->get_item(i)->m_tipo;
        json["log"][i]["ingreso"] = contenedor_log->get_item(i)->m_ingreso;
        json["log"][i]["cambio"] = contenedor_log->get_item(i)->m_cambio;
        json["log"][i]["total"] = contenedor_log->get_item(i)->m_total;
        json["log"][i]["estatus"] = contenedor_log->get_item(i)->m_estatus;
        json["log"][i]["fecha"] = contenedor_log->get_item(i)->m_fecha.format_iso8601();
    }

    return crow::response(json);
}

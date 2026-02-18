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
    auto f_ini = bodyParams["f_ini"].s();
    auto f_fin = bodyParams["f_fin"].s();

    crow::json::wvalue json;
    json["log"] = crow::json::wvalue::list();

    auto contenedor_log = Global::Utility::valida_administrador(req)
                              ? log->get_corte(0, tipo, f_ini, f_fin)
                              : log->get_corte(Global::User::id, tipo, f_ini, f_fin);

    for (size_t i = 0; i < contenedor_log->at("Id").size(); i++)
    {
        json["log"][i]["id"] = std::stoull(contenedor_log->at("Id")[i]);
        json["log"][i]["id_user"] = std::stoull(contenedor_log->at("IdUser")[i]);
        json["log"][i]["tipo"] = contenedor_log->at("Tipo")[i];
        json["log"][i]["descripcion"] = contenedor_log->at("Descripcion")[i];
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
    json["total_rows"] = contenedor_log->at("Id").size();

    // Crear el array como lista
    crow::json::wvalue::list log_list;

    // Llenar el array
    for (size_t i = 0; i < contenedor_log->at("Id").size(); ++i)
    {
        crow::json::wvalue log_entry;
        log_entry["id"] = std::stoull(contenedor_log->at("Id")[i]);
        log_entry["usuario"] = user->get_usuarios(std::stoull(contenedor_log->at("IdUser")[i]))->m_usuario;
        log_entry["tipo"] = contenedor_log->at("Tipo")[i];
        log_entry["descripcion"] = contenedor_log->at("Descripcion")[i];
        log_entry["ingreso"] = std::stoi(contenedor_log->at("Ingreso")[i]);
        log_entry["cambio"] = std::stoi(contenedor_log->at("Cambio")[i]);
        log_entry["total"] = std::stoi(contenedor_log->at("Total")[i]);
        log_entry["estatus"] = contenedor_log->at("Estatus")[i];
        log_entry["fecha"] = contenedor_log->at("Fecha")[i];

        // Anidar detalle_movimiento
        auto bd_detalle = std::make_unique<DetalleMovimiento>();
        auto detalle_movimiento = bd_detalle->get_detalle_movimiento(std::stoull(contenedor_log->at("Id")[i]));

        crow::json::wvalue::list detalle_list;
        if (detalle_movimiento->contains("id"))
            for (size_t j = 0; j < detalle_movimiento->at("id").size(); ++j)
            {
                crow::json::wvalue det;
                // det["id"] = std::stoull(detalle_movimiento->at("id")[j]);
                det["id_log"] = std::stoull(detalle_movimiento->at("id_log")[j]);
                det["tipo_movimiento"] = detalle_movimiento->at("tipo_movimiento")[j];
                det["denominacion"] = std::stoi(detalle_movimiento->at("denominacion")[j]);
                det["cantidad"] = std::stoi(detalle_movimiento->at("cantidad")[j]);
                det["creado_en"] = detalle_movimiento->at("creado_en")[j];

                detalle_list.push_back(std::move(det));
            }

        log_entry["detalle_movimiento"] = std::move(detalle_list);

        // Agregar al array de logs
        log_list.push_back(std::move(log_entry));
    }

    // Asignar el array completo
    json["log"] = std::move(log_list);

    return crow::response(json);
}
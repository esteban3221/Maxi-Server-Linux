#include "controller/log.hpp"
#include "controller/cloud/worker.hpp"

LogData::LogData(crow::SimpleApp &app)
{
    CloudWorker::getInstance().start();
    CROW_ROUTE(app, "/log/movimientos").methods("POST"_method)(sigc::mem_fun(*this, &LogData::get_log));
    CROW_ROUTE(app, "/log/corte_caja").methods("GET"_method)(sigc::mem_fun(*this, &LogData::corte_caja));
    CROW_ROUTE(app, "/log/get_levels").methods("GET"_method)(sigc::mem_fun(*this, &LogData::get_levels));
}

LogData::~LogData()
{
    CloudWorker::getInstance().stop();
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

    auto contenedor_log = Sesion::valida_administrador(req)
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
    Sesion::valida_autorizacion(req, Global::User::Rol::Consulta_Movimientos);

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
        log_entry["metodo"] = contenedor_log->at("Metodo")[i];
        log_entry["ingreso"] = std::stoi(contenedor_log->at("Ingreso")[i]);
        log_entry["cambio"] = std::stoi(contenedor_log->at("Cambio")[i]);
        log_entry["total"] = std::stoi(contenedor_log->at("Total")[i]);
        log_entry["estatus"] = contenedor_log->at("Estatus")[i];
        log_entry["fecha"] = contenedor_log->at("Fecha")[i];

        auto bd_detalle = std::make_unique<DetalleMovimiento>();
        auto detalle_movimiento = bd_detalle->get_detalle_movimiento(std::stoull(contenedor_log->at("Id")[i]));

        crow::json::wvalue::list detalle_list;
        if (detalle_movimiento->contains("id"))
            for (size_t j = 0; j < detalle_movimiento->at("id").size(); ++j)
            {
                crow::json::wvalue det;
                det["id_log"] = std::stoull(detalle_movimiento->at("id_log")[j]);
                det["tipo_movimiento"] = detalle_movimiento->at("tipo_movimiento")[j];
                det["denominacion"] = std::stoi(detalle_movimiento->at("denominacion")[j]);
                det["cantidad"] = std::stoi(detalle_movimiento->at("cantidad")[j]);
                det["creado_en"] = detalle_movimiento->at("creado_en")[j];

                detalle_list.push_back(std::move(det));
            }

        log_entry["detalle_movimiento"] = std::move(detalle_list);
        log_list.push_back(std::move(log_entry));
    }

    // Asignar el array completo
    json["log"] = std::move(log_list);

    return crow::response(json);
}

crow::response LogData::get_levels(const crow::request &req)
{
    bool tiene_pago = Sesion::tiene_permiso(req, Global::User::Rol::Pago);
    bool tiene_consulta = Sesion::tiene_permiso(req, Global::User::Rol::Consulta_Efectivo);
    bool tiene_carga = Sesion::tiene_permiso(req, Global::User::Rol::Carga);
    bool tiene_retirada_cassette = Sesion::tiene_permiso(req, Global::User::Rol::Retirar_Casette);

    if (!tiene_pago && !tiene_consulta && !tiene_carga && !tiene_retirada_cassette) 
        return crow::response(crow::status::UNAUTHORIZED, "Denegado");

    crow::json::wvalue json_final;
    auto &hub = CashHub::instance();
    auto map = hub.obten_ultimo_snapshot_level();

    for (auto const &[device_id, valor] : map)
    {
        auto niveles_hw = valor["levels"];
        std::string tipo_dispositivo = valor["type"].s();
        auto tipo_db = (tipo_dispositivo == "COIN" ? "Level_Coin" : "Level_Bill");
        auto config_db = std::make_unique<LevelCash>(tipo_db)->get_level_cash();

        std::vector<crow::json::wvalue> levels_ofuscados;

        for (size_t i = 0; i < niveles_hw.size(); i++)
        {
            auto item_hw = niveles_hw[i];
            crow::json::wvalue level_item;

            level_item["value"] = item_hw["value"].i();

            level_item["storedInPayout"] = (tiene_pago || tiene_consulta || tiene_carga) 
                ? crow::json::wvalue(item_hw["storedInPayout"].u()) 
                : crow::json::wvalue(nullptr);

            level_item["storedInCashbox"] = tiene_retirada_cassette 
                ? crow::json::wvalue(item_hw["storedInCashbox"].u()) 
                : crow::json::wvalue(nullptr);

            if (i < config_db->get_n_items())
            {
                auto db_item = config_db->get_item(i);
                level_item["Inmovilidad_Min"] = db_item->m_nivel_inmo_min;
                level_item["Inmovilidad"] = db_item->m_nivel_inmo;
                level_item["Inmovilidad_Max"] = db_item->m_nivel_inmo_max;
            }

            levels_ofuscados.push_back(std::move(level_item));
        }

        json_final[device_id]["levels"] = std::move(levels_ofuscados);
        json_final[device_id]["type"] = tipo_dispositivo;
    }

    return crow::response(crow::status::OK, json_final);
}
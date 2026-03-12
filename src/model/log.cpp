#include "model/log.hpp"

Log::Log(/* args */) // : tam_row{0}
{
    // Glib::init();
}

Log::~Log()
{
}

const std::shared_ptr<ResultMap> Log::get_corte(int id_user, const std::string &tipo, const std::string &f_ini, const std::string &f_fin)
{
    auto &database = Database::getInstance();
    std::string query = "SELECT * FROM log WHERE ";

    if (tipo == "Todo" || tipo.empty())
        query += "1=1"; // condición siempre verdadera
    else
        query += "Tipo = '" + tipo + "'";

    if (f_ini.empty() || f_fin.empty())
        query += " AND Fecha >= date('now','localtime')";
    else
        query += " AND Fecha BETWEEN '" + f_ini + "' AND '" + f_fin + "'";

    if (id_user != 0)
        query += " AND IdUser = ?";

    query += " ORDER BY id DESC";

    auto result = (id_user != 0)
                      ? database.sqlite3->command(query, id_user)
                      : database.sqlite3->command(query);
    return result;
}

const std::shared_ptr<ResultMap> Log::get_log(const std::string &tipo, const std::string &f_ini, const std::string &f_fin, int paginacion)
{
    auto &database = Database::getInstance();

    auto query = "SELECT * FROM log WHERE " + (tipo == "Todo" ? "1=1" : "Tipo = '" + tipo + "'") + (f_ini.empty() or f_fin.empty() ? "" : " and Fecha BETWEEN '" + f_ini + "' AND '" + f_fin + "'") +
                 " ORDER BY id DESC LIMIT 100 OFFSET ?";
    return database.sqlite3->command(query, paginacion);
}

size_t Log::insert_log(const Glib::RefPtr<MLog> &list)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("INSERT INTO log VALUES(null,?,?,?,?,?,?,?,?)",
                              list->m_id_user,
                              list->m_tipo.c_str(),
                              list->m_descripcion.c_str(),
                              list->m_ingreso,
                              list->m_cambio,
                              list->m_total,
                              list->m_estatus.c_str(),
                              list->m_fecha.format_iso8601().c_str());

    auto contenedor_data = database.sqlite3->command("SELECT Id FROM log ORDER BY Id DESC LIMIT 1");
    return std::stoull(contenedor_data->at("Id")[0]);
}

void Log::update_log(const Glib::RefPtr<MLog> &list)
{
    auto &database = Database::getInstance();
    auto contenedor_data = database.sqlite3->command("UPDATE log SET idUser = ?, Tipo = ?, Ingreso = ?, Cambio = ?, Total = ?, Estatus = ?, Fecha = ? WHERE Id = ?",
                                                     list->m_id_user,
                                                     list->m_tipo.c_str(),
                                                     list->m_ingreso,
                                                     list->m_cambio,
                                                     list->m_total,
                                                     list->m_estatus.c_str(),
                                                     list->m_fecha.format_iso8601().c_str(),
                                                     list->m_id);
}

crow::json::wvalue Log::json_ticket(Glib::RefPtr<MLog> t_log)
{
    auto user = std::make_unique<Usuarios>();
    crow::json::wvalue data;
    data["ticket"] = crow::json::wvalue::list();

    data["ticket"][0]["id"] = t_log->m_id;
    data["ticket"][0]["usuario"] = user->get_usuarios(t_log->m_id_user)->m_usuario;
    data["ticket"][0]["fecha"] = t_log->m_fecha.format_iso8601();
    data["ticket"][0]["tipo"] = t_log->m_tipo;
    data["ticket"][0]["descripcion"] = t_log->m_descripcion;
    data["ticket"][0]["total"] = t_log->m_total;
    data["ticket"][0]["cambio"] = t_log->m_cambio;
    data["ticket"][0]["ingreso"] = t_log->m_ingreso;
    data["ticket"][0]["estatus"] = t_log->m_estatus;

    return data;
}
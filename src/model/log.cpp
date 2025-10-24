#include "model/log.hpp"

Log::Log(/* args */) : tam_row{0}
{
    Glib::init();
}

Log::~Log()
{
}

const std::shared_ptr<ResultMap> Log::get_corte(int id_user, const std::string &tipo)
{
    auto &database = Database::getInstance();

    return database.sqlite3->command("SELECT * FROM log WHERE " + 
                                    (tipo == "Todo" ? "1=1" : "Tipo = '" + tipo + "'") + 
                                    " AND Fecha >= date('now','localtime') AND IdUser = ? ORDER BY id DESC", /* Global::User::id*/ id_user);
}

Glib::RefPtr<Gio::ListStore<MLog>> Log::get_log(const std::string &tipo, const std::string &f_ini, const std::string &f_fin, int last_id)
{
    auto &database = Database::getInstance();

    std::vector<std::string> conditions;
    std::vector<std::string> params;

    if (tipo != "Todo") {
        conditions.push_back("Tipo = ?");
        params.push_back(tipo);
    }

    if (!f_ini.empty() && !f_fin.empty()) {
        conditions.push_back("Fecha BETWEEN ? AND ?");
        params.push_back(f_ini);
        params.push_back(f_fin);
    }

    if (last_id > 0) {
        conditions.push_back("id < ?");
        params.push_back(std::to_string(last_id));
    }

    std::string where_clause = conditions.empty() ? "1=1" : "";
    for (size_t i = 0; i < conditions.size(); ++i) {
        if (i > 0) where_clause += " AND ";
        where_clause += conditions[i];
    }

    auto count_sql = "SELECT count(*) FROM log WHERE " + where_clause;
    auto contenedor_data = database.sqlite3->command(count_sql, params);
    tam_row = std::stoi(contenedor_data->at("count(*)")[0]);

    auto query = "SELECT * FROM log WHERE " + where_clause + " ORDER BY id DESC LIMIT 100";
    contenedor_data = database.sqlite3->command(query, params);

    auto m_list = Gio::ListStore<MLog>::create();
    for (size_t i = 0; i < contenedor_data->at("Id").size(); i++) {
        m_list->append(MLog::create(
            std::stoull(contenedor_data->at("Id")[i]),
            std::stoull(contenedor_data->at("IdUser")[i]),
            contenedor_data->at("Tipo")[i],
            std::stoi(contenedor_data->at("Ingreso")[i]),
            std::stoi(contenedor_data->at("Cambio")[i]),
            std::stoi(contenedor_data->at("Total")[i]),
            contenedor_data->at("Estatus")[i],
            Glib::DateTime::create_from_iso8601(contenedor_data->at("Fecha")[i])));
    }

    return m_list;
}

size_t Log::insert_log(const Glib::RefPtr<MLog> &list)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("INSERT INTO log VALUES(null,?,?,?,?,?,?,?)",
                              list->m_id_user,
                              list->m_tipo.c_str(),
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
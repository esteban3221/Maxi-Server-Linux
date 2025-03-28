#include "model/log.hpp"

Log::Log(/* args */): tam_row{0}
{
    Glib::init();
}

Log::~Log()
{
}

Glib::RefPtr<Gio::ListStore<MLog>> Log::get_log(const std::string &tipo, const std::string &f_ini, const std::string &f_fin, int paginacion)
{
    auto &database = Database::getInstance();

    auto sql = "SELECT count(*) FROM log WHERE Tipo Like '%" + tipo + "'" + (f_ini.empty() or f_fin.empty() ? "" : " and Fecha BETWEEN '" + f_ini + "' AND '" + f_fin + "'");
    database.sqlite3->command(sql);
    auto contenedor_data = database.sqlite3->get_result();
    auto tam = contenedor_data["count(*)"][0];
    tam_row = std::stoi(tam);

    auto query = "SELECT * FROM log WHERE Tipo Like '%" + tipo + "'" + (f_ini.empty() or f_fin.empty() ? "" : " and Fecha BETWEEN '" + f_ini + "' AND '" + f_fin + "'") +
                              " LIMIT 100 OFFSET ?";
    database.sqlite3->command(query, paginacion);
    auto m_list = Gio::ListStore<MLog>::create();

    contenedor_data = database.sqlite3->get_result();

    for (size_t i = 0; i < contenedor_data["Id"].size(); i++)
    {
        m_list->append(MLog::create(
            std::stoull(contenedor_data["Id"][i]),
            std::stoull(contenedor_data["IdUser"][i]),
            contenedor_data["Tipo"][i],
            std::stoi(contenedor_data["Ingreso"][i]),
            std::stoi(contenedor_data["Cambio"][i]),
            std::stoi(contenedor_data["Total"][i]),
            contenedor_data["Estatus"][i],
            Glib::DateTime::create_from_iso8601(contenedor_data["Fecha"][i])));
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

    database.sqlite3->command("SELECT Id FROM log ORDER BY Id DESC LIMIT 1");
    return std::stoull(database.sqlite3->get_result()["Id"][0]);
}

void Log::update_log(const Glib::RefPtr<MLog> &list)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("UPDATE log SET idUser = ?, Tipo = ?, Ingreso = ?, Cambio = ?, Total = ?, Estatus = ?, Fecha = ? WHERE Id = ?",
                              list->m_id_user,
                              list->m_tipo.c_str(),
                              list->m_ingreso,
                              list->m_cambio,
                              list->m_total,
                              list->m_estatus.c_str(),
                              list->m_fecha.format_iso8601().c_str(),
                              list->m_id);
}
#include "log.hpp"

Log::Log(/* args */)
{
    Glib::init();
}

Log::~Log()
{
}

Glib::RefPtr<Gio::ListStore<MLog>> Log::get_log()
{
    auto &database = Database::getInstance();
    database.sqlite3->command("select * from log");
    auto m_list = Gio::ListStore<MLog>::create();

    auto contenedor_data = database.sqlite3->get_result();

    for (size_t i = 0; i < contenedor_data["id"].size(); i++)
    {
        m_list->append(MLog::create(
            std::stoull(contenedor_data["id"][i]),
            std::stoull(contenedor_data["idUser"][i]),
            contenedor_data["Tipo"][i],
            std::stoi(contenedor_data["Ingreso"][i]),
            std::stoi(contenedor_data["Cambio"][i]),
            std::stoi(contenedor_data["Total"][i]),
            contenedor_data["Estatus"][i],
            Glib::DateTime::create_from_iso8601(contenedor_data["Fecha"][i])));
    }

    return m_list;
}

void Log::insert_log(const MLog &list)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("INSERT INTO from log VALUES(null,?,?,?,?,?,?,?,?)",
                              list.m_id_user,
                              list.m_tipo.c_str(),
                              list.m_ingreso,
                              list.m_cambio,
                              list.m_total,
                              list.m_estatus.c_str(),
                              list.m_fecha.format_iso8601().c_str()
                              );
}

void Log::update_log(const MLog &list)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("UPDATE log SET idUser = ?, Tipo = ?, Ingreso = ?, Cambio = ?, Total = ?, Estatus = ?, Fecha = ? WHERE Id = ?",
                              list.m_id_user,
                              list.m_tipo.c_str(),
                              list.m_ingreso,
                              list.m_cambio,
                              list.m_total,
                              list.m_estatus.c_str(),
                              list.m_fecha.format_iso8601().c_str(),
                              list.m_id
                              );
}

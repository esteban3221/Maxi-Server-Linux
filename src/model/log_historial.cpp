#include "log_historial.hpp"

LogHistorial::LogHistorial(/* args */)
{
}

LogHistorial::~LogHistorial()
{
}

Glib::RefPtr<Gio::ListStore<MLogHistorial>> LogHistorial::get_log_historial()
{
    auto &database = Database::getInstance();
    database.sqlite3->command("select * from log_historial");
    auto m_list = Gio::ListStore<MLogHistorial>::create();

    auto contenedor_data = database.sqlite3->get_result();

    for (size_t i = 0; i < contenedor_data["id"].size(); i++)
    {
        m_list->append(MLogHistorial::create(
            std::stoull(contenedor_data["id"][i]),
            std::stoull(contenedor_data["original_id"][i]),
            std::stoull(contenedor_data["old_id_user"][i]),
            contenedor_data["old_tipo"][i],
            std::stoi(contenedor_data["old_ingreso"][i]),
            std::stoi(contenedor_data["old_cambio"][i]),
            std::stoi(contenedor_data["old_total"][i]),
            contenedor_data["old_status"][i],
            Glib::DateTime::create_from_iso8601(contenedor_data["old_fecha"][i]),

            std::stoull(contenedor_data["new_id_user"][i]),
            contenedor_data["new_tipo"][i],
            std::stoi(contenedor_data["new_ingreso"][i]),
            std::stoi(contenedor_data["new_cambio"][i]),
            std::stoi(contenedor_data["new_total"][i]),
            contenedor_data["new_status"][i],
            Glib::DateTime::create_from_iso8601(contenedor_data["new_fecha"][i]),

            Glib::DateTime::create_from_iso8601(contenedor_data["fecha_modificacion"][i]),
            contenedor_data["usuario_operacion"][i],
            contenedor_data["tipo_cambio"][i]));
    }

    return m_list;
}

/// @brief Inserta a BD (Fecha se debe de pasar normal, esta funcion ya hace la conversion a iso8601)
/// @param list 
void LogHistorial::insert_log_historial(const MLogHistorial &list)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("INSERT INTO log_historial VALUES(null, ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,? )",
                              list.m_original_id,

                              list.m_old_id_user,
                              list.m_old_tipo.c_str(),
                              list.m_old_ingreso,
                              list.m_old_cambio,
                              list.m_old_total,
                              list.m_old_estatus,
                              list.m_old_fecha.format_iso8601().c_str(),

                              list.m_new_id_user,
                              list.m_new_tipo.c_str(),
                              list.m_new_ingreso,
                              list.m_new_cambio,
                              list.m_new_total,
                              list.m_new_estatus,
                              list.m_new_fecha.format_iso8601().c_str(),

                              list.m_fecha_modificacion.format_iso8601().c_str(),
                              list.m_usuario_operacion.c_str(),
                              list.m_tipo_cambio.c_str());
}
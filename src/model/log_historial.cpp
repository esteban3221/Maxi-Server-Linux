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
    auto contenedor_data = database.sqlite3->command("select * from log_historial");
    auto m_list = Gio::ListStore<MLogHistorial>::create();


    for (size_t i = 0; i < contenedor_data->at("id").size(); i++)
    {
        m_list->append(MLogHistorial::create(
            std::stoull(contenedor_data->at("id")[i]),
            std::stoull(contenedor_data->at("original_id")[i]),
            std::stoull(contenedor_data->at("old_id_user")[i]),
            contenedor_data->at("old_tipo")[i],
            std::stoi(contenedor_data->at("old_ingreso")[i]),
            std::stoi(contenedor_data->at("old_cambio")[i]),
            std::stoi(contenedor_data->at("old_total")[i]),
            contenedor_data->at("old_status")[i],
            Glib::DateTime::create_from_iso8601(contenedor_data->at("old_fecha")[i]),

            std::stoull(contenedor_data->at("new_id_user")[i]),
            contenedor_data->at("new_tipo")[i],
            std::stoi(contenedor_data->at("new_ingreso")[i]),
            std::stoi(contenedor_data->at("new_cambio")[i]),
            std::stoi(contenedor_data->at("new_total")[i]),
            contenedor_data->at("new_status")[i],
            Glib::DateTime::create_from_iso8601(contenedor_data->at("new_fecha")[i]),

            Glib::DateTime::create_from_iso8601(contenedor_data->at("fecha_modificacion")[i]),
            contenedor_data->at("usuario_operacion")[i],
            contenedor_data->at("tipo_cambio")[i]));
    }

    return m_list;
}

/// @brief Inserta a BD (Fecha se debe de pasar normal, esta funcion ya hace la conversion a iso8601)
/// @param list 
void LogHistorial::insert_log_historial(const Glib::RefPtr<MLogHistorial> &list)
{
    auto &database = Database::getInstance();
    auto contenedor_data = database.sqlite3->command("INSERT INTO log_historial VALUES(null, ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,? )",
                              list->m_original_id,

                              list->m_old_id_user,
                              list->m_old_tipo.c_str(),
                              list->m_old_ingreso,
                              list->m_old_cambio,
                              list->m_old_total,
                              list->m_old_estatus,
                              list->m_old_fecha.format_iso8601().c_str(),

                              list->m_new_id_user,
                              list->m_new_tipo.c_str(),
                              list->m_new_ingreso,
                              list->m_new_cambio,
                              list->m_new_total,
                              list->m_new_estatus,
                              list->m_new_fecha.format_iso8601().c_str(),

                              list->m_fecha_modificacion.format_iso8601().c_str(),
                              list->m_usuario_operacion.c_str(),
                              list->m_tipo_cambio.c_str());
}
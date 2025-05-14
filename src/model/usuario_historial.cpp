#include "usuario_historial.hpp"

UsuarioHistorial::UsuarioHistorial()
{

}

UsuarioHistorial::~UsuarioHistorial()
{
}

Glib::RefPtr<Gio::ListStore<MUsuarioHistorial>> UsuarioHistorial::get_usuario_historial()
{

    auto &database = Database::getInstance();
    auto contenedor_data = database.sqlite3->command("select * from usuarios_historial");
    auto m_list = Gio::ListStore<MUsuarioHistorial>::create();


    for (size_t i = 0; i < contenedor_data->at("id").size(); i++)
    {
        m_list->append(MUsuarioHistorial::create(
            std::stoull(contenedor_data->at("id")[i]),
            std::stoull(contenedor_data->at("original_id")[i]),
            contenedor_data->at("old_username")[i],
            contenedor_data->at("old_password")[i],
            contenedor_data->at("new_username")[i],
            contenedor_data->at("new_password")[i],
            Glib::DateTime::create_from_iso8601(contenedor_data->at("id")[i]),
            contenedor_data->at("usuario_operacion")[i],
            contenedor_data->at("tipo_cambio")[i]
        ));
    }
    return m_list;
}

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
    database.sqlite3->command("select * from usuarios_historial");
    auto m_list = Gio::ListStore<MUsuarioHistorial>::create();

    auto contenedor_data = database.sqlite3->get_result();

    for (size_t i = 0; i < contenedor_data["id"].size(); i++)
    {
        m_list->append(MUsuarioHistorial::create(
            std::stoull(contenedor_data["id"][i]),
            std::stoull(contenedor_data["original_id"][i]),
            contenedor_data["old_username"][i],
            contenedor_data["old_password"][i],
            contenedor_data["new_username"][i],
            contenedor_data["new_password"][i],
            Glib::DateTime::create_from_iso8601(contenedor_data["id"][i]),
            contenedor_data["usuario_operacion"][i],
            contenedor_data["tipo_cambio"][i]
        ));
    }
    return m_list;
}

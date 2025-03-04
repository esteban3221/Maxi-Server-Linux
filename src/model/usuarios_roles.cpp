#include "usuarios_roles.hpp"

UsuariosRoles::UsuariosRoles(/* args */)
{
}

UsuariosRoles::~UsuariosRoles()
{
}

Glib::RefPtr<Gio::ListStore<MUsuariosRoles>> UsuariosRoles::get_usuario_roles_by_id(size_t id)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("select * from usuario_roles WHERE id_usuario = ?", id);

    auto contenedor_data = database.sqlite3->get_result();
    auto m_list = Gio::ListStore<MUsuariosRoles>::create();

    for (size_t i = 0; i < contenedor_data["id"].size(); i++)
    {
        m_list->append(MUsuariosRoles::create( 
            std::stoull(contenedor_data["id"][i]),
            std::stoull(contenedor_data["id_usuario"][i]),
            std::stoi(contenedor_data["id_rol"][i])
        ));
    }

    return m_list;
}

void UsuariosRoles::update_usuario_roles(size_t id, const Glib::RefPtr<Gio::ListStore<MRoles>> & list_roles)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("DELETE FROM usuario_roles WHERE id = ?", id);

    for (size_t i = 0; i < 19; i++)
            if(list_roles.get()->get_item(i))
                database.sqlite3->command("INSERT INTO usuario_roles values (NULL, ? , ?)", id, i+1);
}

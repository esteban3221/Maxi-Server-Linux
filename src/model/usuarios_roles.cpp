#include "usuarios_roles.hpp"

UsuariosRoles::UsuariosRoles(/* args */)
{
}

UsuariosRoles::~UsuariosRoles()
{
}

Glib::RefPtr<MUsuariosRoles> UsuariosRoles::get_usuario_roles_by_id(size_t id)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("select * from usuarios_roles WHERE id = ?", id);

    auto contenedor_data = database.sqlite3->get_result();

    return MUsuariosRoles::create( 
        std::stoull(contenedor_data["id"][0]),
        std::stoull(contenedor_data["id_usuario"][0]),
        stoi(contenedor_data["id_rol"][0])
    );
}

void UsuariosRoles::update_usuario_roles(size_t id, const Glib::RefPtr<Gio::ListStore<MRoles>> & list_roles)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("DELETE FROM usuarios_roles WHERE id = ?", id);

    for (size_t i = 0; i < 19; i++)
            if(list_roles.get()->get_item(i))
                database.sqlite3->command("INSERT INTO usuario_roles values (NULL, ? , ?)", id, i+1);
}

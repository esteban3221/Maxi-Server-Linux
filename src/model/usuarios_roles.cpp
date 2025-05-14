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
    auto contenedor_data = database.sqlite3->command("select * from usuario_roles WHERE id_usuario = ?", id);

    auto m_list = Gio::ListStore<MUsuariosRoles>::create();

    for (size_t i = 0; i < contenedor_data->at("id").size(); i++)
    {
        m_list->append(MUsuariosRoles::create( 
            std::stoull(contenedor_data->at("id")[i]),
            std::stoull(contenedor_data->at("id_usuario")[i]),
            std::stoi(contenedor_data->at("id_rol")[i])
        ));
    }

    return m_list;
}

void UsuariosRoles::update_usuario_roles(size_t id, const Glib::RefPtr<Gio::ListStore<MRoles>> & list_roles)
{
    auto &database = Database::getInstance();
    auto contenedor_data = database.sqlite3->command("DELETE FROM usuario_roles WHERE id_usuario = ?", id);

    for (size_t i = 0; i < list_roles->get_n_items(); i++)
    {
        auto item = list_roles->get_item(i);
        auto contenedor_data = database.sqlite3->command("INSERT INTO usuario_roles VALUES(null,?,?)",
                                  id,
                                  item->m_id);
    }
                
}

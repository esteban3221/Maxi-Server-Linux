#include "usuarios.hpp"

Usuarios::Usuarios(/* args */)
{
}

Usuarios::~Usuarios()
{
}

Glib::RefPtr<Gio::ListStore<MUsuarios>> Usuarios::get_usuarios()
{
    auto &database = Database::getInstance();
    database.sqlite3->command("select * from usuarios");
    auto m_list = Gio::ListStore<MUsuarios>::create();

    auto contenedor_data = database.sqlite3->get_result();

    for (size_t i = 0; i < contenedor_data["id"].size(); i++)
    {
        m_list->append(MUsuarios::create(
            std::stoull(contenedor_data["id"][i]),
            contenedor_data["username"][i],
            contenedor_data["password"][i]));
    }
    return m_list;
}

void Usuarios::insert_level_cash(const MUsuarios &usuario)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("INSERT INTO usuarios VALUES(null,?,?)",
                              usuario.m_usuario.c_str(),
                              usuario.m_passsword.c_str());
}

void Usuarios::update_level_cash(const MUsuarios &usuario)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("UPDATE usuarios SET username = ?, password = ? WHERE id = ?",
                              usuario.m_usuario.c_str(),
                              usuario.m_passsword.c_str(),
                              usuario.m_id);
}

void Usuarios::delete_level_cash(const MUsuarios &usuario)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("DELETE FROM usuarios WHERE id = ?", usuario.m_id);
    database.sqlite3->command("DELETE FROM usuarios_roles WHERE id = ?", usuario.m_id);
}

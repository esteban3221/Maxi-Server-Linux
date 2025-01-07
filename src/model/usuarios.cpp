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

const std::string &Usuarios::existe_usuario(const std::string &pass) const
{
    auto &database = Database::getInstance();
    database.sqlite3->command("select * from usuarios where password = ?", pass.c_str());

    return database.sqlite3->get_result()["username"][0];
}

size_t Usuarios::insert_usuario(const Glib::RefPtr<MUsuarios> &usuario)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("SELECT * FROM usuarios WHERE username = ? LIMIT 1", usuario->m_usuario.c_str());

        if (database.sqlite3->get_result()["id"].size() > 0)
            throw std::runtime_error("El usuario ya existe");

    database.sqlite3->command("INSERT INTO usuarios VALUES(null,?,?)",
                              usuario->m_usuario.c_str(),
                              usuario->m_passsword.c_str());

    database.sqlite3->command("SELECT id FROM usuarios u order by id desc LIMIT 1");
    return std::stoull(database.sqlite3->get_result()["id"][0]);
}

void Usuarios::update_usuario(const Glib::RefPtr<MUsuarios> &usuario)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("UPDATE usuarios SET username = ?, password = ? WHERE id = ?",
                              usuario->m_usuario.c_str(),
                              usuario->m_passsword.c_str(),
                              usuario->m_id);
}

void Usuarios::delete_usuario(const Glib::RefPtr<MUsuarios> &usuario)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("DELETE FROM usuarios WHERE id = ?", usuario->m_id);
    database.sqlite3->command("DELETE FROM usuarios_roles WHERE id = ?", usuario->m_id);
}

#include "model/roles.hpp"

Rol::Rol(/* args */)
{
}

Rol::~Rol()
{
}

Glib::RefPtr<Gio::ListStore<MRoles>> Rol::get_roles()
{
    auto &database = Database::getInstance();
    database.sqlite3->command("select * from roles");
    auto m_list = Gio::ListStore<MRoles>::create();

    auto contenedor_data = database.sqlite3->get_result();

    for (size_t i = 0; i < contenedor_data["id"].size(); i++)
    {
        m_list->append(MRoles::create(
            std::stoi(contenedor_data["id"][i]),
            contenedor_data["roles"][i]
        ));
    }
    return m_list;
}
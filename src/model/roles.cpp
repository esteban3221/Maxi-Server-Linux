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
    auto contenedor_data = database.sqlite3->command("select * from roles");
    auto m_list = Gio::ListStore<MRoles>::create();


    for (size_t i = 0; i < contenedor_data->at("id").size(); i++)
    {
        m_list->append(MRoles::create(
            std::stoi(contenedor_data->at("id")[i]),
            contenedor_data->at("roles")[i]
        ));
    }
    return m_list;
}
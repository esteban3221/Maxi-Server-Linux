#include "configuracion.hpp"

Configuracion::Configuracion(/* args */)
{
}

Configuracion::~Configuracion()
{
}

Glib::RefPtr<Gio::ListStore<MConfiguracion>> Configuracion::get_conf_data(int inicio, int fin)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("select * from configuracion where id BETWEEN ? AND ? ORDER BY id ASC", inicio, fin);
    auto m_list = Gio::ListStore<MConfiguracion>::create();

    auto contenedor_data = database.sqlite3->get_result();

    for (size_t i = 0; i < contenedor_data["id"].size(); i++)
    {
        m_list->append(MConfiguracion::create(
            std::stoi(contenedor_data["id"][i]),
            contenedor_data["descripcion"][i],
            contenedor_data["valor"][i]
        ));
    }

    return m_list;
}

void Configuracion::update_conf(const Glib::RefPtr<MConfiguracion> &conf)
{

    auto &database = Database::getInstance();
    database.sqlite3->command("UPDATE configuracion set descripcion = ?, valor = ? WHERE id = ?", 
        conf->m_descripcion.c_str(),
        conf->m_valor.c_str(),
        conf->m_id);
}

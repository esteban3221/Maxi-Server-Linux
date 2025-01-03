#include "pago_pendiente.hpp"

PagoPendiente::PagoPendiente(/* args */)
{
}

PagoPendiente::~PagoPendiente()
{
}

Glib::RefPtr<Gio::ListStore<MPagoPendiente>> PagoPendiente::get_log_historial()
{
    auto &database = Database::getInstance();
    database.sqlite3->command("select * from pagoPendiente");
    auto m_list = Gio::ListStore<MPagoPendiente>::create();

    auto contenedor_data = database.sqlite3->get_result();

    for (size_t i = 0; i < contenedor_data["Id"].size(); i++)
    {
        m_list->append(MPagoPendiente::create(
            std::stoull(contenedor_data["Id"][i]),
            std::stoull(contenedor_data["IdLog"][i]),
            stoi(contenedor_data["Remanente"][i]),
            contenedor_data["Estatus"][i]));
    }
    return m_list;
}

void PagoPendiente::update_log_historial(const MPagoPendiente &list)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("UPDATE pagoPendiente set IdLog = ?, Remanente = ?, Estatus = ? WHERE Id = ?",
                              list.m_id_log,
                              list.m_remanente,
                              list.m_status.c_str(),
                              list.m_id);
}
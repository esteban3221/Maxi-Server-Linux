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
    auto contenedor_data = database.sqlite3->command("select * from pagoPendiente");
    auto m_list = Gio::ListStore<MPagoPendiente>::create();


    for (size_t i = 0; i < contenedor_data->at("Id").size(); i++)
    {
        m_list->append(MPagoPendiente::create(
            std::stoull(contenedor_data->at("Id")[i]),
            std::stoull(contenedor_data->at("IdLog")[i]),
            stoi(contenedor_data->at("Remanente")[i]),
            contenedor_data->at("Estatus")[i]));
    }
    return m_list;
}

void PagoPendiente::update_log_historial(const Glib::RefPtr<MPagoPendiente> &list)
{
    auto &database = Database::getInstance();
    auto contenedor_data = database.sqlite3->command("UPDATE pagoPendiente set IdLog = ?, Remanente = ?, Estatus = ? WHERE Id = ?",
                              list->m_id_log,
                              list->m_remanente,
                              list->m_status.c_str(),
                              list->m_id);
}
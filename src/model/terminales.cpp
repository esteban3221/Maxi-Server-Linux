#include "model/terminales.hpp"

OTerminales::OTerminales(/* args */)
{
}

OTerminales::~OTerminales()
{
}

void OTerminales::inserta(const Glib::RefPtr<MTerminales> &terminal)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("INSERT INTO terminales_pago (id, tipo, alias, modo, predeterminado, descripcion, fecha_creado) VALUES (?, ?, ?, ?, ?, ?, ?)",
                              terminal->m_id,
                              terminal->m_tipo,
                              terminal->m_alias,
                              terminal->m_modo,
                              terminal->m_predeterminado ? 1 : 0,
                              terminal->m_descripcion,
                              terminal->m_fecha_creado.format_iso8601());
}

void OTerminales::edita(const Glib::RefPtr<MTerminales> &terminal)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("UPDATE terminales_pago SET tipo = ?, alias = ?, modo = ?, predeterminado = ?, descripcion = ? WHERE Id = ?",
                              terminal->m_tipo,
                              terminal->m_alias,
                              terminal->m_modo,
                              terminal->m_predeterminado ? 1 : 0,
                              terminal->m_descripcion,
                              terminal->m_id);
}

void OTerminales::elimina(const Glib::ustring &id)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("DELETE FROM terminales_pago WHERE Id = ?", id);
}

Glib::RefPtr<MTerminales> OTerminales::get_by_id(const Glib::ustring &id)
{
    auto &database = Database::getInstance();
    auto contenedor_data = database.sqlite3->command("SELECT * FROM terminales_pago WHERE Id = ?", id);

    if (contenedor_data->at("Id").empty())
        return nullptr;

    return MTerminales::create(
        contenedor_data->at("id")[0],
        contenedor_data->at("tipo")[0],
        contenedor_data->at("alias")[0],
        contenedor_data->at("modo")[0],
        std::stoi(contenedor_data->at("predeterminado")[0]) == 1,
        contenedor_data->at("descripcion")[0],
        Glib::DateTime::create_from_iso8601(contenedor_data->at("fecha_creado")[0]));
}

std::shared_ptr<ResultMap> OTerminales::get_all()
{
    auto &database = Database::getInstance();
    return database.sqlite3->command("SELECT * FROM terminales_pago");
}
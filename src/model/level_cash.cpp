#include "level_cash.hpp"

LevelCash::LevelCash(const std::string &tipo) : TIPO(tipo)
{
}

LevelCash::~LevelCash()
{
}

Glib::RefPtr<Gio::ListStore<MLevelCash>> LevelCash::get_level_cash()
{

    auto &database = Database::getInstance();
    database.sqlite3->command("select * from " + TIPO);
    auto m_list_log = Gio::ListStore<MLevelCash>::create();

    auto contenedor_data = database.sqlite3->get_result();

    for (size_t i = 0; i < contenedor_data["Denominacion"].size(); i++)
    {
        m_list_log->append(MLevelCash::create(
            std::stoi(contenedor_data["Denominacion"][i]),
            std::stoi(contenedor_data["Cant_Alm"][i]),
            std::stoi(contenedor_data["Cant_Recy"][i]),
            std::stoi(contenedor_data["Nivel_inmo"][i])));
    }

    return m_list_log;
}

void LevelCash::update_level_cash(const MLevelCash &level)
{
    auto &database = Database::getInstance();
    database.sqlite3->command("update ? set Denominacion = ?, Cant_Alm = ? Cant_Recy = ? Nivel_inmo = ?",
                              TIPO.c_str(),
                              level.m_denominacion,
                              level.m_cant_alm,
                              level.m_cant_recy,
                              level.m_nivel_inmo);
}

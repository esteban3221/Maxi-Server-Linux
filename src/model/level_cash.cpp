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
    auto contenedor_data = database.sqlite3->command("select * from " + TIPO);
    auto m_list_log = Gio::ListStore<MLevelCash>::create();

    for (size_t i = 0; i < contenedor_data->at("Denominacion").size(); i++)
    {
        m_list_log->append(MLevelCash::create(
            std::stoi(contenedor_data->at("Denominacion")[i]),
            std::stoi(contenedor_data->at("Cant_Alm")[i]),
            std::stoi(contenedor_data->at("Cant_Recy")[i]),
            std::stoi(contenedor_data->at("Nivel_Inmo_Min")[i]),
            std::stoi(contenedor_data->at("Nivel_Inmo")[i]),
            std::stoi(contenedor_data->at("Nivel_Inmo_Max")[i]),
            0));
    }

    return m_list_log;
}

void LevelCash::update_level_cash(const Glib::RefPtr<MLevelCash> &level)
{
    auto &database = Database::getInstance();
    auto contenedor_data = database.sqlite3->command("update " + TIPO + " set Cant_Alm = ?, Cant_Recy = ?, Nivel_Inmo_Min = ?, Nivel_Inmo = ?, Nivel_Inmo_Max = ? where Denominacion = ?",
                              level->m_cant_alm,
                              level->m_cant_recy,
                              level->m_nivel_inmo_min,
                              level->m_nivel_inmo,
                              level->m_nivel_inmo_max,
                              level->m_denominacion);
}

void LevelCash::update_nivel_inmo(int denominacion, int nivel_inmo_min, int nivel_inmo, int nivel_inmo_max)
{
    auto &database = Database::getInstance();
    auto contenedor_data = database.sqlite3->command("update " + TIPO + " set Nivel_Inmo_Min = ?, Nivel_Inmo = ?, Nivel_Inmo_Max = ? where Denominacion = ?",
                              nivel_inmo_min,
                              nivel_inmo,
                              nivel_inmo_max,
                              denominacion);
}

#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <glibmm.h>
#include <giomm.h>

#include "coneccion.hpp"

class MLevelCash : public Glib::Object
{
public:
    u_int16_t m_denominacion;
    u_int16_t m_cant_alm;
    u_int16_t m_cant_recy;
    u_int16_t m_nivel_inmo;

    static Glib::RefPtr<MLevelCash> create(u_int16_t denominacion, u_int16_t cant_alm, u_int16_t cant_recy, u_int16_t nivel_inmo)
    {
        return Glib::make_refptr_for_instance<MLevelCash>(new MLevelCash(denominacion, cant_alm, cant_recy, nivel_inmo));
    }

protected:
    MLevelCash(u_int16_t denominacion, u_int16_t cant_alm, u_int16_t cant_recy, u_int16_t nivel_inmo)
        : m_denominacion(denominacion), m_cant_alm(cant_alm), m_cant_recy(cant_recy), m_nivel_inmo(nivel_inmo)
    {
    }
}; // ModelColumns

class LevelCash
{
private:
    const std::string TIPO;

public:
    LevelCash(const std::string &tipo);
    ~LevelCash();

    // std::vector<std::shared_ptr<Model::LevelCash_t>> get_log();
    Glib::RefPtr<Gio::ListStore<MLevelCash>> get_level_cash();
    void update_level_cash(const Glib::RefPtr<MLevelCash> &level);
};
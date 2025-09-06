#pragma once
#include <iostream>
#include <gtkmm.h>
#include <vector>
#include <atomic>
#include <chrono>
#include <thread>

#include "configuracion.hpp"

class VCarrousel : public Gtk::Stack
{
private:
    
    std::vector<std::unique_ptr<Gtk::Image>> vec_pages;
    size_t mili_seconds_move;
    bool mueve_carrousel();
    void obtener_tiempo_carrousel();
    void on_dropdown_directory_time_selected();
    void init(void);

    enum class DropdownTime
    {
        NONE,
        FIVE,
        TEN,
        FIFTEEN,
        THIRTY,
        ONE_MIN
    };
public:
    VCarrousel(/* args */);
    ~VCarrousel();

    void clear_pages();
    void init_imgs(void);

    void set_image_pages(const std::vector<Gtk::Image*> &vec_images);
    size_t &property_miliseconds_move(void){return this->mili_seconds_move;}
    std::vector<std::string> listar_contenido(const std::string& ruta_carpeta);
    sigc::connection conn;
};

namespace Global
{
    namespace Widget
    {
        extern std::string default_home;
        extern VCarrousel *v_carrousel;
    }
}
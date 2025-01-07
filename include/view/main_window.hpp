#pragma once
#include <gtkmm.h>
#include "global.hpp"
#include "session.hpp"

class Main_window : public Gtk::Window
{
private:
    Sesion sesion;
public:
    Main_window(/* args */);
    ~Main_window();
};

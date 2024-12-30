
#include <iostream>
#include "main_window.hpp"
#include "log.hpp"

int main(int argc, char *argv[])
{   
    auto app = Gtk::Application::create("org.gtkmm.maxicajero.base");
    return app->make_window_and_run<Main_window>(argc, argv);   
}

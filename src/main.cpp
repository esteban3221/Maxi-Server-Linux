
#include <iostream>
#include "controller/main_window.hpp"

int main(int argc, char *argv[])
{   
    auto app = Gtk::Application::create("org.gtkmm.maxicajero.base");
    return app->make_window_and_run<MainWindow>(argc, argv);   
}

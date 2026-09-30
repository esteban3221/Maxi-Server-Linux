#include <iostream>
#include <filesystem>
#include <limits.h>
#include <unistd.h>
#include <glibmm.h>
#include "controller/main_window.hpp"

void clear_update_flag()
{
    char result_[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result_, PATH_MAX);
    if (count <= 0)
        return;

    std::string current_executable_path(result_, count);
    std::string flag_path = current_executable_path + ".update_in_progress";

    std::error_code ec;
    if (std::filesystem::exists(flag_path, ec))
    {
        std::filesystem::remove(flag_path, ec);
        if (!ec)
        {
            g_message("Health Check: Nueva versión validada con éxito. Archivo de flag eliminado.");
        }
    }
}

int main(int argc, char *argv[])
{
    auto app = Gtk::Application::create("org.gtkmm.maxicajero.base");

    app->signal_startup().connect([&app]()
                                  { clear_update_flag(); });

    return app->make_window_and_run<MainWindow>(argc, argv);
}
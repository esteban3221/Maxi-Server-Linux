#include "main_window.hpp"

Main_window::Main_window(/* args */)
{
    // iniciacion de apirest
    Rest_api rest;

    this->set_title("Ventana de Prueba");
    this->set_size_request(400,400);
    this->set_default_size(400,400);

    Gtk::Label label("Hola que hace");
    label.set_halign(Gtk::Align::CENTER);
    label.set_valign(Gtk::Align::CENTER);

    this->set_child(label);
}

Main_window::~Main_window()
{
    Global::Rest::app.stop();
    Global::Rest::future.get();
}
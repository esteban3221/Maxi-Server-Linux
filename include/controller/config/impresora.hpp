#pragma once
#include "view/config/impresora.hpp"
#include "configuracion.hpp"
#include "global.hpp"

class Impresora : public VImpresora
{
private:
    void init_data();

    void activa_impresion(Gtk::ListBoxRow *row);
    void activa_visualizacion(Gtk::ListBoxRow *row);
    void activa_test(Gtk::ListBoxRow *row);

    void estado_checkbutton();

    std::string test_text_impresion();
    void actualiza_buffer();

    Glib::RefPtr<Gtk::TextBuffer> text_buffer = Gtk::TextBuffer::create();;

public:
    Impresora(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Impresora();
};
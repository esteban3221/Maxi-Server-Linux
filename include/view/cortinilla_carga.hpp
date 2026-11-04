#pragma once

#include <gtkmm.h>

class ViewCarga : public Gtk::Box
{
public:
    ViewCarga(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder);
    virtual ~ViewCarga();

    enum Modo {
        Tarjeta,
        Efectivo,
        Retirada
    };

    void modo(Modo modo);
    Gtk::Button* v_btn_cancel;

private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    // Widgets del template
    Gtk::Image* v_img_main;
    Gtk::Label* v_lbl_main;
    Gtk::Label* v_lbl_sub;
    Gtk::Spinner* v_spinner;
};

namespace View
{
   extern const char *ui_cortinilla_carga;
}
#pragma once

#include <gtkmm.h>

class ViewCarga : public Gtk::Box
{
public:
    ViewCarga(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder);
    virtual ~ViewCarga();

    void modo(bool is_efectivo = false);

private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    // Widgets del template
    Gtk::Image* v_img_main;
    Gtk::Label* v_lbl_main;
    Gtk::Label* v_lbl_sub;
    Gtk::Spinner* v_spinner;
    Gtk::Button* v_btn_cancel_tarjeta;

};

namespace View
{
   extern const char *ui_cortinilla_carga;
}
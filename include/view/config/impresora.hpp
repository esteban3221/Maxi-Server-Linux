#pragma once
#include <gtkmm.h>

class VImpresora : public Gtk::Box
{
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    Gtk::TextView *text_ticket = nullptr;
    Gtk::ListBox *list_config = nullptr;
    Gtk::ListBox *list_config_visualizacion = nullptr;
    Gtk::ListBox *list_config_test = nullptr;
    
public:
    VImpresora(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VImpresora();
};

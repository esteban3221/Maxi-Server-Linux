#pragma once
#include <gtkmm.h>

class VImpresora : public Gtk::Box
{
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    Gtk::TextView *v_text_ticket = nullptr;
    Gtk::ListBox *v_list_config = nullptr;
    Gtk::ListBox *v_list_config_visualizacion = nullptr;
    Gtk::ListBox *v_list_config_test = nullptr;

    Gtk::Switch *v_switch_impresion = nullptr;

    Gtk::CheckButton *v_check_config[6]{nullptr};
    
public:
    VImpresora(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VImpresora();
};

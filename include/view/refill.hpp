#pragma once
#include <gtkmm.h>
#include "global.hpp"
#include "level_cash.hpp"

class VRefill : public Gtk::Box
{
protected:
    
    Gtk::Label *v_lbl_total_parcial_monedas,
                *v_lbl_total_parcial_billetes,
                *v_lbl_total,
                *v_lbl_total_billetes,
                *v_lbl_total_monedas;
    
    Gtk::Button *v_btn_deten = nullptr;

    void on_setup_label(const Glib::RefPtr<Gtk::ListItem> &list_item);
    void on_bind_deno(const Glib::RefPtr<Gtk::ListItem> &list_item);
    void on_bind_alm(const Glib::RefPtr<Gtk::ListItem> &list_item);
    void on_bind_recy(const Glib::RefPtr<Gtk::ListItem> &list_item);
    void on_bind_ingreso(const Glib::RefPtr<Gtk::ListItem> &list_item);
    void on_bind_inmo(const Glib::RefPtr<Gtk::ListItem> &list_item);
public:
    VRefill(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VRefill();
};
#pragma once
#include <gtkmm.h>

class VRefill : public Gtk::Box
{
protected:
    Gtk::ColumnView *v_tree_reciclador_monedas, *v_tree_reciclador_billetes;
    Gtk::Label *v_lbl_total_parcial_monedas,
                *v_lbl_total_parcial_billetes,
                *v_lbl_total,
                *v_lbl_total_billetes,
                *v_lbl_total_monedas;

public:
    VRefill(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VRefill();
};
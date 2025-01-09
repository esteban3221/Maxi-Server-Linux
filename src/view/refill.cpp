#include "view/refill.hpp"

VRefill::VRefill(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : Gtk::Box(cobject)
{
    v_tree_reciclador_monedas = refBuilder->get_widget<Gtk::ColumnView>("treeRecicladorMonedas");
    v_tree_reciclador_billetes = refBuilder->get_widget<Gtk::ColumnView>("treeRecicladorBilletes");

    v_lbl_total_parcial_monedas = refBuilder->get_widget<Gtk::Label>("lblTotalMXN1");
    v_lbl_total_parcial_billetes = refBuilder->get_widget<Gtk::Label>("lblTotalMXN2");
    v_lbl_total_monedas = refBuilder->get_widget<Gtk::Label>("lblTotalMXN3");
    v_lbl_total_billetes = refBuilder->get_widget<Gtk::Label>("lblTotalMXN4");
    v_lbl_total = refBuilder->get_widget<Gtk::Label>("lblTotalMXN5");
}

VRefill::~VRefill()
{
}
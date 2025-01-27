#include "view/refill.hpp"

VRefill::VRefill(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : Gtk::Box(cobject)
{
    Global::Widget::Refill::v_tree_reciclador_monedas  = refBuilder->get_widget<Gtk::ColumnView>("treeRecicladorMonedas");
    Global::Widget::Refill::v_tree_reciclador_billetes = refBuilder->get_widget<Gtk::ColumnView>("treeRecicladorBilletes");

    v_lbl_total_parcial_monedas = refBuilder->get_widget<Gtk::Label>("lblTotalMXN1");
    v_lbl_total_parcial_billetes = refBuilder->get_widget<Gtk::Label>("lblTotalMXN2");
    v_lbl_total_monedas = refBuilder->get_widget<Gtk::Label>("lblTotalMXN4");
    v_lbl_total_billetes = refBuilder->get_widget<Gtk::Label>("lblTotalMXN5");
    v_lbl_total = refBuilder->get_widget<Gtk::Label>("lblTotalMXN3");
}


void VRefill::on_setup_label(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    list_item->set_child(*Gtk::make_managed<Gtk::Label>("", Gtk::Align::END));
}

void VRefill::on_bind_deno(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MLevelCash>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_denominacion));
}

void VRefill::on_bind_alm(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MLevelCash>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_cant_alm));
}

void VRefill::on_bind_recy(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MLevelCash>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_cant_recy));
}

void VRefill::on_bind_ingreso(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MLevelCash>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_ingreso));
}

void VRefill::on_bind_inmo(const Glib::RefPtr<Gtk::ListItem> &list_item)
{
    auto col = std::dynamic_pointer_cast<MLevelCash>(list_item->get_item());
    auto label = dynamic_cast<Gtk::Label *>(list_item->get_child());

    label->set_text(Glib::ustring::format(col->m_nivel_inmo));
}


VRefill::~VRefill()
{
}
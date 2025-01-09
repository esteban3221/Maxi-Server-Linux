#pragma once
#include "view/refill.hpp"
#include "level_cash.hpp"

class Refill : public VRefill
{
private:
    void init_data(Gtk::ColumnView *vcolumn, const std::string &tabla);
    void on_setup_label(const Glib::RefPtr<Gtk::ListItem>& list_item);
    void on_bind_deno(const Glib::RefPtr<Gtk::ListItem>& list_item);
    void on_bind_alm(const Glib::RefPtr<Gtk::ListItem>& list_item);
    void on_bind_recy(const Glib::RefPtr<Gtk::ListItem>& list_item);
    void on_bind_inmo(const Glib::RefPtr<Gtk::ListItem>& list_item);
public:
    Refill(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Refill();
};

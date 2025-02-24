#pragma once
#include <gtkmm.h>

class VWifi : public Gtk::Box
{
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    Gtk::EditableLabel *lbl_red[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};
    Gtk::Button *btnRed = nullptr;

public:
    VWifi(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VWifi();
};
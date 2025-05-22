#pragma once
#include <gtkmm.h>

class VWifi : public Gtk::Box
{
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    Gtk::EditableLabel *v_lbl_red[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};
    Gtk::Button *v_btn_red = nullptr;

public:
    VWifi(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VWifi();
};

namespace View
{
   extern const char *ui_wifi;
}

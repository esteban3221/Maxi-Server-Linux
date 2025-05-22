#pragma once
#include <gtkmm.h>

class VInfo : public Gtk::Box
{
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    Gtk::EditableLabel *v_list_info_system[5]{nullptr};

public:
    VInfo(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VInfo();
};

namespace View
{
   extern const char *ui_info;
}

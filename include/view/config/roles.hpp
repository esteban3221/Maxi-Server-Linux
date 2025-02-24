#pragma once
#include <gtkmm.h>

class VRoles : public Gtk::Box
{
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    Gtk::CheckButton *v_rol[19]{nullptr};
    Gtk::ColumnView *v_tree_usuarios;

public:
    VRoles(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VRoles();
};
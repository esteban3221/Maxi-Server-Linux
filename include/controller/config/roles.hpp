#pragma once
#include "view/config/roles.hpp"
#include "configuracion.hpp"
#include "model/usuarios.hpp"
#include "model/usuarios_roles.hpp"

#include "global.hpp"

class Roles : public VRoles
{
private:
    /* data */

    void init_data();

    void on_setup_label(const Glib::RefPtr<Gtk::ListItem> &list_item);
    void on_bind_name(const Glib::RefPtr<Gtk::ListItem> &list_item);

    void on_active_list(guint num);

public:
    Roles(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Roles();
};
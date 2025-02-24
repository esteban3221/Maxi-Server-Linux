#pragma once
#include "view/config/roles.hpp"

class Roles : public VRoles
{
private:
    /* data */
public:
    Roles(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Roles();
};
#pragma once
#include "view/config/general.hpp"

class General : public VGeneral
{
private:
    /* data */
public:
    General(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~General();
};

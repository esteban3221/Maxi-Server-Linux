#pragma once
#include "view/config/info.hpp"

class Info : public VInfo
{
private:
    /* data */
public:
    Info(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Info();
};
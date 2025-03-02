#pragma once
#include "view/config/info.hpp"
#include "global.hpp"

class Info : public VInfo
{
private:
    void init_data();
public:
    Info(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Info();
};
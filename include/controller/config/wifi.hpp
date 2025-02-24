#pragma once
#include "view/config/wifi.hpp"

class Wifi : public VWifi
{
private:
    /* data */
public:
    Wifi(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Wifi();
};
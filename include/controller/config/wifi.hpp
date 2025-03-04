#pragma once
#include "view/config/wifi.hpp"
#include "global.hpp"

class Wifi : public VWifi
{
private:
    void init_data();
public:
    Wifi(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Wifi();
};
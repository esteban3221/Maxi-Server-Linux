#pragma once
#include "view/config/wifi.hpp"
#include "global.hpp"

#include <vector>

class Wifi : public VWifi
{
private:
    void init_data();
    void on_btn_redes_clicked();
    void on_btn_regresar_clicked();
    void on_show_map_wifi();
    void on_list_box_wifi_row_activated(Gtk::ListBoxRow* row);
    void init_dialog(const std::string &wifi_ssid);

    void escanea_redes();

    struct NetworkInfo {
        std::string ssid;
        std::string signal;
        std::string security;
        std::string bars;
        bool is_connected;
    };

    std::vector<NetworkInfo> obten_datos_redes(const std::string &data);

public:
    Wifi(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Wifi();
};
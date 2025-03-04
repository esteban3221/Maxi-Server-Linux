#include "controller/config/wifi.hpp"

Wifi::Wifi(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VWifi(cobject, refBuilder) 
{
    this->v_btn_red->signal_clicked().connect([]() { std::system("nm-connection-editor &"); });
    init_data();
}

Wifi::~Wifi()
{
}

void Wifi::init_data()
{
    Glib::signal_timeout().connect([this]() -> bool
    {
        const std::string SSID = Global::System::exec("nmcli -t -f name,device connection show --active | grep -v ':lo' | cut -d':' -f1");
        
        const std::string IP_WLAN = Global::System::exec(
        "for iface in wlan0 wlp2s0; do "
        "  ip=$(ip -o -4 addr show $iface 2>/dev/null | awk '{print $4}' | cut -d/ -f1); "
        "  if [ ! -z \"$ip\" ]; then echo $ip; break; fi; "
        "done");

        const std::string MAC_WLAN = Global::System::exec(
            "for iface in wlan0 wlp2s0; do "
            "  mac=$(cat /sys/class/net/$iface/address 2>/dev/null); "
            "  if [ ! -z \"$mac\" ]; then echo $mac; break; fi; "
            "done");

        const std::string IP_ETHERNET = Global::System::exec(
            "for iface in eth0 end0 enp0s31f6 enp0s20f0u1u4; do "
            "  ip=$(ip -o -4 addr show $iface 2>/dev/null | awk '{print $4}' | cut -d/ -f1); "
            "  if [ ! -z \"$ip\" ]; then echo $ip; break; fi; "
            "done");

        const std::string MAC_ETHERNET = Global::System::exec(
            "for iface in eth0 end0 enp0s31f6 enp0s20f0u1u4; do "
            "  mac=$(cat /sys/class/net/$iface/address 2>/dev/null); "
            "  if [ ! -z \"$mac\" ]; then echo $mac; break; fi; "
            "done");
            
            // Especifico de la RPI 4
            this->v_lbl_red[4]->set_text(SSID.empty() ? "No conectado." : SSID);
            this->v_lbl_red[0]->set_text(IP_WLAN.empty() ? "- - - -" : IP_WLAN);
            this->v_lbl_red[1]->set_text(MAC_WLAN.empty() ? "" : MAC_WLAN);

            this->v_lbl_red[2]->set_text(IP_ETHERNET.empty() ? "No conectado." : IP_ETHERNET);
            this->v_lbl_red[3]->set_text(MAC_ETHERNET.empty() ? "" : MAC_ETHERNET);

        return true;
    }, 5000);
}
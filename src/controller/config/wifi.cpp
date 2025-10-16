#include "controller/config/wifi.hpp"

Wifi::Wifi(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VWifi(cobject, refBuilder) 
{
    this->v_btn_red->signal_clicked().connect([]() { std::system("nm-connection-editor &"); });
    this->v_btn_redes->signal_clicked().connect(sigc::mem_fun(*this, &Wifi::on_btn_redes_clicked));
    this->v_btn_regresar->signal_clicked().connect(sigc::mem_fun(*this, &Wifi::on_btn_regresar_clicked));
    v_list_box_wifi->signal_map().connect(sigc::mem_fun(*this, &Wifi::on_show_map_wifi));
    init_data();
}

Wifi::~Wifi()
{
}

std::vector<Wifi::NetworkInfo> Wifi::obten_datos_redes(const std::string &data)
{
    std::istringstream stream(data);
    std::string line;
    std::vector<NetworkInfo> networks;
    
    while (std::getline(stream, line)) {
        // Cada línea representa una red diferente
        // Dividir la línea por ':' para obtener los campos
        std::istringstream linestream(line);
        std::vector<std::string> fields;
        std::string field;
        
        while (std::getline(linestream, field, ':')) {
            fields.push_back(field);
        }
        
        // Verificar que tenemos al menos los campos básicos
        if (fields.size() >= 5) {
            NetworkInfo network;
            network.ssid = fields[0];
            network.signal = fields[1];
            network.security = fields[2];
            network.bars = fields[3];
            network.is_connected = (fields[4] == "*");
            
            networks.push_back(network);
        }
    }
    return networks;
}

void Wifi::on_show_map_wifi()
{
    v_list_box_wifi->remove_all();
    escanea_redes();
}

void Wifi::escanea_redes()
{
    std::string stdout_output, stderr_output;
    int exit_status = 0;
    
    try {
        Glib::spawn_sync("",
            std::vector<std::string>{"sh", "-c", "nmcli -t -f SSID,SIGNAL,SECURITY,BARS,IN-USE dev wifi"},
            Glib::SpawnFlags::SEARCH_PATH,
            {},
            &stdout_output,
            &stderr_output,
            &exit_status
        );
        
        if (exit_status != 0) {
            throw std::runtime_error("Command failed: " + stderr_output);
        }
        
    } catch (const Glib::Error& error) {
        throw std::runtime_error("Glib spawn error: " + std::string(error.what()));
    }


    for (auto &&i : obten_datos_redes(stdout_output))
    {
        auto row = Gtk::make_managed<VWifi::VWifiRow>(i.ssid, i.security + " - " + i.signal + "%");
        v_list_box_wifi->append(*row);

        //std::cout << "SSID: " << i.ssid << ", Signal: " << i.signal << ", Security: " << i.security << ", Bars: " << i.bars << ", Connected: " << (i.is_connected ? "Yes" : "No") << std::endl;
    }
}

void Wifi::on_btn_redes_clicked()
{
    v_stack_wifi->set_visible_child("redes_wifi");
}

void Wifi::on_btn_regresar_clicked()
{
    v_stack_wifi->set_visible_child("status_red");
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
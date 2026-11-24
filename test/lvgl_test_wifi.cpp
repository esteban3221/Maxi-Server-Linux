#include "adw_lvgl.h"

#include <array>
#include <memory>
#include <string>
#include <vector>

struct MockNetwork {
    std::string ssid;
    std::string signal;
    std::string security;
    bool is_connected;
};

class WifiPreview {
public:
    WifiPreview()
        : scr_(lv_screen_active())
        , header_(scr_, "Configuración de Red", false)
        , v_stack_wifi_(scr_)
        , v_btn_back_header_(header_, LV_SYMBOL_LEFT, adw::BtnVariant::Flat)
    {   
        header_.pack_start(v_btn_back_header_);
        scr_.init_keyboard();
        build_status_page();
        build_networks_page();
        connect_signals();
    }

private:
    void build_status_page() {
        auto page_status = v_stack_wifi_.add_page("status_red", 14);
        adw::ScrollView scroll(page_status, 18);

        // Sección Inalámbrica
        adw::Box hdr_wifi(scroll, adw::Orientation::Horizontal, 10);
        hdr_wifi.set_align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        adw::Label lbl_wifi(hdr_wifi, "Inalámbrica", ADW_FONT_TITLE_2);
        lbl_wifi.set_flex_grow(1);
        v_btn_redes_ = adw::Button(hdr_wifi, "Conectar Wi-Fi", adw::BtnVariant::Suggested);

        adw::ListBox list_wifi(scroll);
        v_lbl_red_[4] = list_wifi.add_value_row("Red", "Sucursal_5G");
        v_lbl_red_[0] = list_wifi.add_value_row("Dirección IP", "192.168.1.48", true, ADW_COLOR_FG, ADW_FONT_MONO);
        v_lbl_red_[1] = list_wifi.add_value_row("Dirección MAC", "DC:A6:32:1A:4F:90", false, ADW_COLOR_FG, ADW_FONT_MONO);

        // Sección Ethernet
        adw::Box hdr_eth(scroll, adw::Orientation::Horizontal, 10);
        hdr_eth.set_margin_top(10);
        hdr_eth.set_align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        adw::Label lbl_eth(hdr_eth, "Ethernet", ADW_FONT_TITLE_2);
        lbl_eth.set_flex_grow(1);
        v_btn_red_ = adw::Button(hdr_eth, "Gestionar conexiones", adw::BtnVariant::Warning);

        adw::ListBox list_eth(scroll);
        v_lbl_red_[2] = list_eth.add_value_row("Dirección IP", "No conectado.");
        v_lbl_red_[3] = list_eth.add_value_row("Dirección MAC", "DC:A6:32:1A:4F:91", false, ADW_COLOR_FG, ADW_FONT_MONO);
    }

    void build_networks_page() {
        auto page_redes = v_stack_wifi_.add_page("redes_wifi", 10);

        adw::Box hdr_redes(page_redes, adw::Orientation::Horizontal, 10);
        hdr_redes.set_padding(14);
        hdr_redes.set_align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        adw::Label lbl_disp(hdr_redes, "Redes disponibles", ADW_FONT_TITLE_2);
        lbl_disp.set_flex_grow(1);
        v_btn_refresh_ = adw::IconButton(hdr_redes, LV_SYMBOL_REFRESH, adw::BtnVariant::Default);

        adw::ScrollView scroll_redes(page_redes, 18);
        v_list_box_wifi_ = adw::ListBox(scroll_redes);
    }

    void connect_signals() {
        v_btn_red_.signal_clicked().connect([]() {
            adw::Toast::show("Abriendo gestor de conexiones...");
        });

        v_btn_redes_.signal_clicked().connect([this]() {
            v_stack_wifi_.set_visible_child("redes_wifi");
        });

        v_btn_back_header_.signal_clicked().connect([this]() {
            v_stack_wifi_.set_visible_child("status_red");
        });


        v_btn_refresh_.signal_clicked().connect([this]() {
            simular_escaneo();
        });

        v_stack_wifi_.on_page_shown([this](const std::string& page) {
            if (page == "redes_wifi") {
                simular_escaneo();
            }
        });
    }

    void simular_escaneo() {
        loading_.show("Escaneando redes...");

        // Timer de un solo disparo (retorna false) para simular la latencia del escaneo
        scan_timer_.start(600, [this]() {
            loading_.close();
            poblar_redes({
                {"Sucursal_5G",      "92", "WPA2", true},
                {"Maxicajero_Admin", "78", "WPA2", false},
                {"Invitados_Local",  "54", "WPA2", false},
                {"",                 "41", "WPA3", false},
            });
            return false;
        });
    }

    void poblar_redes(const std::vector<MockNetwork>& redes) {
        v_list_box_wifi_.remove_all();

        for (size_t i = 0; i < redes.size(); ++i) {
            const auto& net = redes[i];
            std::string titulo = net.ssid.empty() ? "[Red Oculta]" : net.ssid;
            std::string sub    = net.security + " - " + net.signal + "%";
            std::string estado = net.is_connected ? "Conectada" : "";
            bool divisor       = (i + 1 < redes.size());

            v_list_box_wifi_.add_nav_row(
                LV_SYMBOL_WIFI, titulo, sub, estado,
                [this, ssid = titulo]() { abrir_dialogo_password(ssid); },
                divisor);
        }
    }

    void abrir_dialogo_password(const std::string& ssid) {
        adw::PromptDialog::show(
            "Conectar Wi-Fi",
            "Ingresa la contraseña para la red: " + ssid,
            "Contraseña",
            true,
            "Cancelar",
            "Conectar",
            [this, ssid](std::string password) {
                if (password.empty()) {
                    adw::Toast::show("Contraseña vacía");
                    return;
                }
                v_lbl_red_[4].set_value(ssid);
                v_lbl_red_[0].set_value("192.168.1.52");
                v_stack_wifi_.set_visible_child("status_red");
                adw::Toast::show("Conectado a " + ssid);
            });
    }

    adw::Screen scr_;
    adw::HeaderBar header_;
    adw::Stack v_stack_wifi_;
    std::array<adw::ValueRow, 5> v_lbl_red_;
    adw::Button v_btn_red_;
    adw::Button v_btn_redes_;
    adw::IconButton v_btn_back_header_;
    adw::IconButton v_btn_refresh_;
    adw::ListBox v_list_box_wifi_;
    adw::LoadingOverlay loading_;
    adw::Timer scan_timer_;
};

static std::unique_ptr<WifiPreview> g_app;

static void ui_init(void) {
    adw::UiDispatcher::init();
    g_app.reset(); // Limpia timers/overlays previos si el previsualizador recarga en vivo
    g_app = std::make_unique<WifiPreview>();
}

#ifdef LVGL_LIVE_PREVIEW
extern "C" void lvgl_live_preview_init(void) {
    ui_init();
}
#endif
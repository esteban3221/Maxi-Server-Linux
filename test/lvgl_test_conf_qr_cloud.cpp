#include "adw_lvgl.h"

#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <string>

enum class CloudStatus {
    Desconectado,
    EsperandoVinculo,
    Conectado
};

// ============================================================================
// VISTA MAXI CLOUD (equivalente directo a src/view/config/qr_cloud.cpp)
// ============================================================================
class VQrCloud {
public:
    explicit VQrCloud(adw::Widget& parent)
        : split_(parent, adw::Orientation::Horizontal, 18)
    {
        split_.set_flex_grow(1);
        split_.set_padding(18);

        build_left_qr_panel();
        build_right_info_panel();
    }

    void actualizar_qr(const std::string& url) {
        if (url.empty()) return;
#if LV_USE_QRCODE
        if (qr_obj_) {
            lv_qrcode_update(qr_obj_, url.c_str(), static_cast<uint32_t>(url.size()));
        }
#else
        LV_UNUSED(url);
#endif
    }

    void set_uuid(adw::Str uuid) {
        v_label_uuid_.set_value(uuid.empty() ? "Cargando..." : uuid);
    }

    void set_pin(adw::Str pin) {
        adw::Str val = pin.empty() ? "----" : pin;
        v_label_pin_.set_value(val);
        lbl_pin_badge_.set_text(val);
    }

    void set_estado(CloudStatus st, adw::Str texto_custom = "") {
        lv_color_t col = ADW_COLOR_ERROR_TEXT;
        const char* txt = "Desconectado";

        switch (st) {
        case CloudStatus::Conectado:
            col = ADW_COLOR_SUCCESS_TEXT;
            txt = "Conectado";
            break;
        case CloudStatus::EsperandoVinculo:
            col = ADW_COLOR_WARNING_TEXT;
            txt = "Esperando vínculo...";
            break;
        case CloudStatus::Desconectado:
        default:
            col = ADW_COLOR_ERROR_TEXT;
            txt = "Desconectado";
            break;
        }

        v_label_status_.set_value(texto_custom.empty() ? txt : texto_custom.ptr);
        v_label_status_.set_value_color(col);
    }

    void set_canal(uint32_t idx) {
        v_dropdown_canal_.set_selected(idx);
    }

    uint32_t get_canal() const {
        return v_dropdown_canal_.get_selected();
    }

    adw::SignalProxy<uint32_t> signal_canal_changed() {
        return v_dropdown_canal_.signal_changed();
    }

private:
    void build_left_qr_panel() {
        // Tarjeta izquierda dedicada al QR + PIN visible de lejos
        adw::Box card(split_, adw::Orientation::Vertical, 12);
        card.set_size_request(248, lv_pct(100));
        card.set_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        card.set_padding(16);
        lv_obj_set_style_bg_color(card.raw(), ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(card.raw(), ADW_OPA_CARD_BG, 0);
        lv_obj_set_style_radius(card.raw(), 14, 0);
        lv_obj_set_style_border_width(card.raw(), 1, 0);
        lv_obj_set_style_border_color(card.raw(), ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(card.raw(), 45, 0);

        // Marco blanco redondeado (quiet zone obligatorio sobre tema oscuro)
        constexpr int32_t kQrSize = 180;
        lv_obj_t* quiet_zone = lv_obj_create(card.raw());
        lv_obj_remove_style_all(quiet_zone);
        adw_set_scrollable(quiet_zone, false);
        lv_obj_set_size(quiet_zone, kQrSize + 24, kQrSize + 24);
        lv_obj_set_style_bg_color(quiet_zone, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(quiet_zone, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(quiet_zone, 16, 0);
        lv_obj_set_style_shadow_width(quiet_zone, 16, 0);
        lv_obj_set_style_shadow_color(quiet_zone, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_shadow_opa(quiet_zone, 80, 0);

#if LV_USE_QRCODE
        qr_obj_ = lv_qrcode_create(quiet_zone);
        lv_qrcode_set_size(qr_obj_, kQrSize);
        // Mismo azul corporativo de tu Cairo: rgba(0.1, 0.14, 0.49) -> #1A247D
        lv_qrcode_set_dark_color(qr_obj_, lv_color_hex(0x1A247D));
        lv_qrcode_set_light_color(qr_obj_, lv_color_white());
        lv_obj_center(qr_obj_);
#else
        lv_obj_t* ph = lv_label_create(quiet_zone);
        lv_label_set_text(ph, "Activar\nLV_USE_QRCODE 1\nen lv_conf.h");
        lv_obj_set_style_text_align(ph, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(ph, lv_color_hex(0x1A247D), 0);
        lv_obj_set_style_text_font(ph, ADW_FONT_CAPTION, 0);
        lv_obj_center(ph);
#endif

        adw::Label lbl_hint(card, "Escanea con tu celular", ADW_FONT_CAPTION, ADW_COLOR_FG);
        lbl_hint.set_color(ADW_COLOR_FG, ADW_OPA_DIM);

        // Píldora inferior con el PIN en monoespaciado grande
        lv_obj_t* pin_pill = lv_obj_create(card.raw());
        lv_obj_remove_style_all(pin_pill);
        adw_set_scrollable(pin_pill, false);
        lv_obj_set_size(pin_pill, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_radius(pin_pill, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(pin_pill, ADW_COLOR_ACCENT_TEXT, 0);
        lv_obj_set_style_bg_opa(pin_pill, 28, 0);
        lv_obj_set_style_pad_hor(pin_pill, 18, 0);
        lv_obj_set_style_pad_ver(pin_pill, 5, 0);

        adw::Widget pill_w(pin_pill);
        lbl_pin_badge_ = adw::Label(pill_w, "----", ADW_FONT_MONO_LG, ADW_COLOR_ACCENT_TEXT);
    }

    void build_right_info_panel() {
        adw::ScrollView right_scroll(split_, 2);
        right_scroll.set_flex_grow(1);
        lv_obj_set_height(right_scroll.raw(), lv_pct(100));
        lv_obj_set_style_pad_row(right_scroll.raw(), 12, 0);

        adw::Label lbl_titulo(right_scroll, "Maxi Cloud", ADW_FONT_TITLE_2, ADW_COLOR_FG);

        adw::Label lbl_sub(
            right_scroll,
            "Para tener tu Maxi Server en la nube, escanea el código QR con tu celular y sigue los pasos. "
            "Si ya tienes cuenta de Maxi Cloud, inicia sesión y agrega este equipo.",
            ADW_FONT_CAPTION,
            ADW_COLOR_FG);
        lbl_sub.set_color(ADW_COLOR_FG, ADW_OPA_DIM);
        lbl_sub.set_hexpand(true);
        lbl_sub.set_wrap(true);
        lv_obj_set_style_text_line_space(lbl_sub.raw(), 3, 0);

        // Lista Adwaita con los 4 campos de tu vista original
        list_info_ = adw::BoxedList(right_scroll);

        v_label_uuid_ = adw::ValueRow(
            list_info_, "UUID", "", "Cargando...",
            ADW_COLOR_FG, ADW_FONT_MONO, true);

        v_label_status_ = adw::ValueRow(
            list_info_, "Estado", "", "Desconectado",
            ADW_COLOR_ERROR_TEXT, ADW_FONT_HEADING, true);

        v_label_pin_ = adw::ValueRow(
            list_info_, "PIN de vinculación", "", "----",
            ADW_COLOR_ACCENT_TEXT, ADW_FONT_MONO, true);

        v_dropdown_canal_ = adw::ComboRow(
            list_info_,
            "Canal de actualizaciones",
            "",
            "LTS (Estable)\nTest (Pruebas)",
            0,
            false);
        lv_obj_set_width(v_dropdown_canal_.raw(), 168);
    }

    adw::Box split_;
    lv_obj_t* qr_obj_ = nullptr;
    adw::Label lbl_pin_badge_;

    adw::BoxedList list_info_;
    adw::ValueRow v_label_uuid_;
    adw::ValueRow v_label_status_;
    adw::ValueRow v_label_pin_;
    adw::ComboRow v_dropdown_canal_;
};

// ============================================================================
// SIMULADOR INTERACTIVO PARA LVGL LIVE PREVIEW
// ============================================================================
class QrCloudPreviewHost {
public:
    QrCloudPreviewHost()
        : scr_(lv_screen_active())
        , header_(scr_, "Maxi Cloud", true)
        , vista_(scr_)
    {
        btn_nuevo_ = adw::Button(header_, "Nuevo PIN", adw::BtnVariant::Default);
        btn_sim_   = adw::Button(header_, "Simular vínculo", adw::BtnVariant::Suggested);
        header_.pack_end(btn_nuevo_);
        header_.pack_end(btn_sim_);

        // Datos iniciales de prueba
        vista_.set_uuid("9F4A-28C1-B703-E15D");
        generar_nueva_sesion();

        header_.signal_back_clicked().connect([]() {
            adw::Toast::show("Regresando a Configuración...", 1600);
        });

        btn_nuevo_.signal_clicked().connect([this]() {
            conectado_ = false;
            btn_sim_.set_label("Simular vínculo");
            generar_nueva_sesion();
            adw::Toast::show("QR y PIN regenerados", 1800);
        });

        btn_sim_.signal_clicked().connect([this]() {
            conectado_ = !conectado_;
            if (conectado_) {
                vista_.set_estado(CloudStatus::Conectado);
                btn_sim_.set_label("Desconectar");
                adw::Toast::show("Equipo vinculado a Maxi Cloud", 2200);
            } else {
                vista_.set_estado(CloudStatus::Desconectado);
                btn_sim_.set_label("Simular vínculo");
                adw::Toast::show("Sesión de nube desconectada", 1800);
            }
        });

        vista_.signal_canal_changed().connect([](uint32_t idx) {
            adw::Toast::show(idx == 0 ? "Canal: LTS (Estable)" : "Canal: Test (Pruebas)", 1800);
        });
    }

private:
    void generar_nueva_sesion() {
        static constexpr const char* kPins[] = {"482-915", "731-042", "519-684", "204-873"};
        const char* pin = kPins[seq_++ % 4];
        std::string url = std::string("https://cloud.maxicajero.mx/pair?uuid=9F4A-28C1-B703-E15D&pin=") + pin;

        vista_.set_pin(pin);
        vista_.actualizar_qr(url);
        vista_.set_estado(CloudStatus::EsperandoVinculo);
    }

    adw::Screen scr_;
    adw::HeaderBar header_;
    adw::Button btn_nuevo_;
    adw::Button btn_sim_;
    VQrCloud vista_;
    size_t seq_ = 0;
    bool conectado_ = false;
};

static std::unique_ptr<QrCloudPreviewHost> g_qr_app;

static void ui_init(void) {
    adw::UiDispatcher::init();
    g_qr_app.reset();
    g_qr_app = std::make_unique<QrCloudPreviewHost>();
}

#ifdef LVGL_LIVE_PREVIEW
extern "C" void lvgl_live_preview_init(void) {
    ui_init();
}
#endif
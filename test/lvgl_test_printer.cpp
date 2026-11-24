#include "adw_lvgl.h"

#include <array>
#include <memory>
#include <sstream>
#include <string>

class VImpresoraPreview {
public:
    VImpresoraPreview()
        : scr_(lv_screen_active())
        , header_(scr_, "Configuración de Impresora", true)
        , split_(scr_, adw::Orientation::Horizontal, 16)
    {
        // El contenedor dividido ocupa todo el alto bajo el HeaderBar
        split_.set_flex_grow(1);
        split_.set_padding(16);

        build_left_controls();
        build_right_ticket_preview();
        connect_signals();
        refresh_state();
    }

private:
    void build_left_controls() {
        left_scroll_ = adw::ScrollView(split_, 4);
        left_scroll_.set_flex_grow(1);
        lv_obj_set_height(left_scroll_.raw(), lv_pct(100));
        lv_obj_set_style_pad_row(left_scroll_.raw(), 16, 0);

        // ---- 1. Activar ----
        list_config_ = adw::BoxedList(left_scroll_, "Activar");
        v_switch_impresion_ = adw::SwitchRow(
            list_config_,
            "Activar Impresión",
            "Emitir ticket automáticamente al finalizar venta",
            true,
            false);

        // ---- 2. Visualización (6 campos exactos de tu vista original) ----
        list_visualizacion_ = adw::BoxedList(left_scroll_, "Visualización");
        static constexpr std::array<const char*, 6> kLabels = {
            "Mensaje de Agradecimiento",
            "Fecha",
            "Dirección",
            "RFC",
            "Vendedor",
            "Contacto"
        };

        for (size_t i = 0; i < kLabels.size(); ++i) {
            bool divisor = (i + 1 < kLabels.size());
            v_check_config_[i] = adw::CheckRow(list_visualizacion_, kLabels[i], "", true, divisor);
        }

        // ---- 3. Test ----
        list_test_ = adw::BoxedList(left_scroll_, "Test");
        row_test_print_ = adw::NavRow(
            list_test_,
            LV_SYMBOL_FILE,
            "Impresión de Prueba",
            "Envía un ticket de diagnóstico a la impresora",
            "Probar",
            false);
    }

    void build_right_ticket_preview() {
        // Columna derecha fija para alojar un ticket de 32 columnas en Ubuntu Mono 16
        adw::Box right_col(split_, adw::Orientation::Vertical, 10);
        right_col.set_size_request(330, lv_pct(100));

        // Cabecera de la vista previa + píldora de estado
        adw::Box hdr(right_col, adw::Orientation::Horizontal, 8);
        hdr.set_align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        adw::Label lbl_prev(hdr, "Vista previa", ADW_FONT_HEADING);
        lbl_prev.set_flex_grow(1);

        status_badge_ = lv_obj_create(hdr.raw());
        lv_obj_remove_style_all(status_badge_);
        adw_set_scrollable(status_badge_, false);
        lv_obj_set_size(status_badge_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_radius(status_badge_, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_pad_hor(status_badge_, 10, 0);
        lv_obj_set_style_pad_ver(status_badge_, 4, 0);
        lv_obj_set_style_bg_opa(status_badge_, 40, 0);

        lbl_status_ = lv_label_create(status_badge_);
        lv_obj_set_style_text_font(lbl_status_, ADW_FONT_CAPTION, 0);

        // Contenedor scrollable del papel térmico
        ticket_scroll_ = adw::ScrollView(right_col, 0);
        lv_obj_set_height(ticket_scroll_.raw(), lv_pct(100));
        lv_obj_set_style_bg_color(ticket_scroll_.raw(), lv_color_hex(0xF4F3EE), 0);
        lv_obj_set_style_bg_opa(ticket_scroll_.raw(), LV_OPA_COVER, 0);
        lv_obj_set_style_radius(ticket_scroll_.raw(), 10, 0);
        lv_obj_set_style_pad_hor(ticket_scroll_.raw(), 16, 0);
        lv_obj_set_style_pad_ver(ticket_scroll_.raw(), 14, 0);
        lv_obj_set_style_border_width(ticket_scroll_.raw(), 1, 0);
        lv_obj_set_style_border_color(ticket_scroll_.raw(), ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(ticket_scroll_.raw(), 60, 0);
        lv_obj_set_style_shadow_width(ticket_scroll_.raw(), 16, 0);
        lv_obj_set_style_shadow_offset_y(ticket_scroll_.raw(), 4, 0);
        lv_obj_set_style_shadow_color(ticket_scroll_.raw(), ADW_COLOR_SHADE, 0);
        lv_obj_set_style_shadow_opa(ticket_scroll_.raw(), 90, 0);

        v_text_ticket_ = adw::Label(ticket_scroll_, "", ADW_FONT_MONO, lv_color_hex(0x1E1E22));
        v_text_ticket_.set_hexpand(true);
        v_text_ticket_.set_wrap(true);
        lv_obj_set_style_text_line_space(v_text_ticket_.raw(), 4, 0);
    }

    void connect_signals() {
        header_.signal_back_clicked().connect([]() {
            adw::Toast::show("Regresando a Configuración...");
        });

        v_switch_impresion_.signal_toggled().connect([this](bool enabled) {
            refresh_state();
            adw::Toast::show(enabled ? "Impresión activada" : "Impresión desactivada", 1800);
        });

        for (auto& check : v_check_config_) {
            check.signal_toggled().connect([this](bool) {
                update_ticket_preview();
            });
        }

        row_test_print_.signal_activated().connect([this]() {
            if (!v_switch_impresion_.get_active()) return;
            loading_.show("Enviando ticket de prueba...");
            print_timer_.start(800, [this]() {
                loading_.close();
                adw::Toast::show("Ticket de prueba impreso correctamente");
                return false;
            });
        });
    }

    void refresh_state() {
        bool enabled = v_switch_impresion_.get_active();

        for (auto& check : v_check_config_) {
            check.set_sensitive(enabled);
        }
        row_test_print_.set_sensitive(enabled);
        lv_obj_set_style_opa(ticket_scroll_.raw(), enabled ? LV_OPA_COVER : ADW_OPA_DIM, 0);

        lv_color_t badge_col = enabled ? ADW_COLOR_SUCCESS_TEXT : ADW_COLOR_ERROR_TEXT;
        lv_obj_set_style_bg_color(status_badge_, badge_col, 0);
        lv_obj_set_style_text_color(lbl_status_, badge_col, 0);
        lv_label_set_text(lbl_status_, enabled ? "Activa · 80mm" : "Desactivada");

        update_ticket_preview();
    }

    void update_ticket_preview() {
        // Índices 0..5: Agradecimiento, Fecha, Dirección, RFC, Vendedor, Contacto
        std::ostringstream ss;
        ss << "      MAXICAJERO S.A.\n";
        if (v_check_config_[3].get_active()) ss << "     RFC: MXC240101AB1\n";
        if (v_check_config_[2].get_active()) ss << "  Av. Principal #123, Centro\n";
        if (v_check_config_[5].get_active()) ss << "      Tel: 55-1234-5678\n";
        ss << "------------------------------\n";
        if (v_check_config_[1].get_active()) ss << "Fecha: 24/11/2026 14:59\n";
        if (v_check_config_[4].get_active()) ss << "Vendedor: Caja 01 (Admin)\n";
        ss << "Folio: #004821\n";
        ss << "------------------------------\n";
        ss << "Pago servicio         $150.00\n";
        ss << "Recibido              $200.00\n";
        ss << "Cambio                 $50.00\n";
        ss << "------------------------------\n";
        if (v_check_config_[0].get_active()) {
            ss << "\n   ¡Gracias por su compra!\n";
            ss << "      Vuelva pronto :)\n";
        }

        v_text_ticket_.set_text(ss.str());
    }

    adw::Screen scr_;
    adw::HeaderBar header_;
    adw::Box split_;
    adw::ScrollView left_scroll_;
    adw::BoxedList list_config_;
    adw::SwitchRow v_switch_impresion_;
    adw::BoxedList list_visualizacion_;
    std::array<adw::CheckRow, 6> v_check_config_;
    adw::BoxedList list_test_;
    adw::NavRow row_test_print_;

    lv_obj_t* status_badge_ = nullptr;
    lv_obj_t* lbl_status_   = nullptr;
    adw::ScrollView ticket_scroll_;
    adw::Label v_text_ticket_;

    adw::LoadingOverlay loading_;
    adw::Timer print_timer_;
};

static std::unique_ptr<VImpresoraPreview> g_impresora_app;

static void ui_init(void) {
    adw::UiDispatcher::init();
    g_impresora_app.reset();
    g_impresora_app = std::make_unique<VImpresoraPreview>();
}

#ifdef LVGL_LIVE_PREVIEW
extern "C" void lvgl_live_preview_init(void) {
    ui_init();
}
#endif
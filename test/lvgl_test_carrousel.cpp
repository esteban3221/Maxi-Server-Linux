#include "adw_lvgl.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifndef LVGL_LIVE_PREVIEW
#include <filesystem>
#endif

enum class DropdownTime {
    NONE = 0,
    FIVE,
    TEN,
    FIFTEEN,
    THIRTY,
    ONE_MIN
};

// ============================================================================
// VISTA CARROUSEL REUTILIZABLE (reemplazo directo de src/view/carrousel.cpp)
// ============================================================================
class VCarrousel {
public:
    explicit VCarrousel(adw::Widget& parent)
        : carousel_(parent, true)
    {
        carousel_.signal_tap().connect([this]() { on_img_clicked(); });
    }

    void set_on_exit(std::function<void()> cb) {
        on_exit_ = std::move(cb);
    }

    // Equivalente a obtener_tiempo_carrousel() sin acoplarse directo a SQLite
    void set_interval(DropdownTime option) {
        switch (option) {
        case DropdownTime::FIVE:    interval_ms_ = 5000;  break;
        case DropdownTime::TEN:     interval_ms_ = 10000; break;
        case DropdownTime::FIFTEEN: interval_ms_ = 15000; break;
        case DropdownTime::THIRTY:  interval_ms_ = 30000; break;
        case DropdownTime::ONE_MIN: interval_ms_ = 60000; break;
        case DropdownTime::NONE:
        default:                    interval_ms_ = 0;     break;
        }
        apply_autoplay();
    }

    void set_interval_ms(uint32_t ms) {
        interval_ms_ = ms;
        apply_autoplay();
    }

    bool should_start_in_carrousel() const noexcept {
        return interval_ms_ > 0 && carousel_.count() > 0;
    }

    void clear_pages() {
        carousel_.stop_autoplay();
        carousel_.clear_slides();
    }

    // Carga todas las imágenes de una carpeta (Linux POSIX o SD/LittleFS en ESP-IDF)
    // drive_prefix corresponde a la letra configurada en LV_FS_POSIX_LETTER / LV_FS_STDIO_LETTER
    void load_from_folder(const std::string& ruta_carpeta, const std::string& drive_prefix = "A:") {
        clear_pages();
#ifndef LVGL_LIVE_PREVIEW
        for (const auto& ruta : listar_contenido(ruta_carpeta)) {
            carousel_.add_image_slide(drive_prefix + ruta, true);
        }
#else
        LV_UNUSED(ruta_carpeta);
        LV_UNUSED(drive_prefix);
#endif
        apply_autoplay();
    }

    adw::Box add_custom_slide() {
        return carousel_.add_slide();
    }

    void start() { apply_autoplay(); }
    void stop()  { carousel_.stop_autoplay(); }
    void next()  { carousel_.next(); }
    size_t count() const noexcept { return carousel_.count(); }

    static std::vector<std::string> listar_contenido(const std::string& ruta_carpeta) {
        std::vector<std::string> archivos;
#ifndef LVGL_LIVE_PREVIEW
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(ruta_carpeta, ec)) {
            if (!entry.is_regular_file()) continue;
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bin") {
                archivos.push_back(entry.path().string());
            }
        }
        std::sort(archivos.begin(), archivos.end());
#else
        LV_UNUSED(ruta_carpeta);
#endif
        return archivos;
    }

private:
    void apply_autoplay() {
        if (interval_ms_ > 0 && carousel_.count() > 1) {
            carousel_.start_autoplay(interval_ms_);
        } else {
            carousel_.stop_autoplay();
        }
    }

    void on_img_clicked() {
        constexpr uint32_t kTapWindowMs = 2500;
        constexpr int kRequiredTaps     = 6; // Mismo umbral que tu `count_click_image > 5`

        if (lv_tick_elaps(last_tap_tick_) > kTapWindowMs) {
            count_click_image_ = 0;
        }
        last_tap_tick_ = lv_tick_get();
        ++count_click_image_;

        if (count_click_image_ >= kRequiredTaps) {
            count_click_image_ = 0;
            if (on_exit_) on_exit_();
        }
    }

    adw::Carousel carousel_;
    std::function<void()> on_exit_;
    uint32_t interval_ms_   = 5000;
    uint32_t last_tap_tick_ = 0;
    int count_click_image_  = 0;
};

// ============================================================================
// SIMULADOR INTERACTIVO PARA LVGL LIVE PREVIEW (sin archivos en disco)
// ============================================================================
class CarrouselPreviewHost {
public:
    CarrouselPreviewHost()
        : scr_(lv_screen_active())
        , main_stack_(scr_)
    {
        // Página "10": Carrusel protector de pantalla (mismo ID que tu v_main_stack)
        auto page_carrousel = main_stack_.add_page("10", 0);
        carrousel_ = std::make_unique<VCarrousel>(page_carrousel);

        add_promo_slide(
            lv_color_hex(0x162436), ADW_COLOR_ACCENT_TEXT, LV_SYMBOL_CHARGE,
            "Paga aquí sin hacer filas",
            "Aceptamos efectivo con cambio exacto y pagos con tarjeta.");

        add_promo_slide(
            lv_color_hex(0x162A22), ADW_COLOR_SUCCESS_TEXT, LV_SYMBOL_OK,
            "Recargas y Servicios 24/7",
            "Transacciones rápidas con comprobante térmico al instante.");

        add_promo_slide(
            lv_color_hex(0x2B2216), ADW_COLOR_WARNING_TEXT, LV_SYMBOL_BELL,
            "Desliza o toca 6 veces",
            "Deslizar cambia de anuncio; 6 toques rápidos regresan al menú principal.");

        carrousel_->set_on_exit([this]() {
            carrousel_->stop();
            main_stack_.set_visible_child("0");
            adw::Toast::show("Carrusel desbloqueado -> Home (\"0\")", 2200);
        });

        // En el preview usamos 4s para apreciar la transición rápido
        carrousel_->set_interval_ms(4000);

        // Página "0": Home simulado para regresar al carrusel y probar intervalos
        auto page_home = main_stack_.add_page("0", 0);
        adw::HeaderBar hdr(page_home, "Menú Principal (Página \"0\")", false);
        adw::Page body(page_home, 500);

        adw::BoxedList list(body, "Configuración de Carrusel");
        combo_time_ = adw::ComboRow(
            list,
            "Intervalo de cambio",
            "Tiempo entre cada imagen del protector",
            "Desactivado\n5 segundos\n10 segundos\n15 segundos\n30 segundos\n1 minuto",
            1,
            false);

        btn_ver_ = adw::Button(body, "Mostrar Carrusel (Página \"10\")",
                               adw::BtnVariant::Suggested, true);
        btn_ver_.set_hexpand(true);
        btn_ver_.set_height(44);
        btn_ver_.signal_clicked().connect([this]() {
            auto opt = static_cast<DropdownTime>(combo_time_.get_selected());
            carrousel_->set_interval(opt);
            main_stack_.set_visible_child("10");
            carrousel_->start();
        });
    }

private:
    void add_promo_slide(lv_color_t bg, lv_color_t accent,
                         const char* symbol, const char* title, const char* subtitle) {
        adw::Box slide = carrousel_->add_custom_slide();
        lv_obj_set_style_bg_color(slide.raw(), bg, 0);
        lv_obj_set_style_bg_opa(slide.raw(), LV_OPA_COVER, 0);
        lv_obj_set_style_pad_all(slide.raw(), 36, 0);
        lv_obj_set_style_pad_row(slide.raw(), 14, 0);

        // Insignia circular
        lv_obj_t* badge = lv_obj_create(slide.raw());
        lv_obj_remove_style_all(badge);
        adw_set_scrollable(badge, false);
        adw_set_clickable(badge, false);
        lv_obj_set_size(badge, 76, 76);
        lv_obj_set_style_radius(badge, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(badge, accent, 0);
        lv_obj_set_style_bg_opa(badge, 38, 0);

        adw::Widget badge_w(badge);
        adw::Label ico(badge_w, symbol, ADW_FONT_ICONS, accent);
        adw_set_clickable(ico.raw(), false);
        lv_obj_center(ico.raw());

        adw::Label lbl_t(slide, title, ADW_FONT_TITLE_1, ADW_COLOR_FG);
        lbl_t.set_hexpand(true);
        lbl_t.set_wrap(true);
        lv_obj_set_style_text_align(lbl_t.raw(), LV_TEXT_ALIGN_CENTER, 0);
        adw_set_clickable(lbl_t.raw(), false);

        adw::Label lbl_s(slide, subtitle, ADW_FONT_BODY, ADW_COLOR_FG);
        lbl_s.set_color(ADW_COLOR_FG, 200);
        lbl_s.set_hexpand(true);
        lbl_s.set_wrap(true);
        lv_obj_set_style_text_align(lbl_s.raw(), LV_TEXT_ALIGN_CENTER, 0);
        adw_set_clickable(lbl_s.raw(), false);
    }

    adw::Screen scr_;
    adw::Stack main_stack_;
    std::unique_ptr<VCarrousel> carrousel_;
    adw::ComboRow combo_time_;
    adw::Button btn_ver_;
};

static std::unique_ptr<CarrouselPreviewHost> g_carrousel_app;

static void ui_init(void) {
    adw::UiDispatcher::init();
    g_carrousel_app.reset();
    g_carrousel_app = std::make_unique<CarrouselPreviewHost>();
}

#ifdef LVGL_LIVE_PREVIEW
extern "C" void lvgl_live_preview_init(void) {
    ui_init();
}
#endif
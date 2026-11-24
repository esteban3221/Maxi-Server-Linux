#include "adw_lvgl.h"

#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>

// ============================================================================
// VISTA BASE REUTILIZABLE (reemplazo directo de src/view/base.nip.cpp)
// ============================================================================
class VBaseNip {
public:
    explicit VBaseNip(adw::Widget& parent)
        : root_(parent, adw::Orientation::Vertical, 0)
    {
        root_.set_size_request(lv_pct(100), lv_pct(100));
        root_.set_flex_grow(1);
        root_.set_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        root_.set_padding(14);

        // Columna centrada que luce bien en 800x480, 1024x600 o vertical
        container_ = adw::Box(root_, adw::Orientation::Vertical, 12);
        container_.set_size_request(470, lv_pct(100));
        lv_obj_set_style_max_height(container_.raw(), 456, 0);

        build_top_bar();
        build_display();
        build_keypad();

        set_modo_monto(false);
    }

    virtual ~VBaseNip() = default;

    // Equivalente exacto a tu VBaseNip::set_modo_monto(bool)
    void set_modo_monto(bool es_monto) {
        es_monto_ = es_monto;
        peek_pin_ = false;
        buffer_.clear();
        clear_error_state();

        lbl_titulo_.set_text(es_monto_ ? "Ingrese el monto" : "Acceso con NIP");
        lbl_mode_badge_.set_text(es_monto_ ? "MXN" : "PIN");
        btn_peek_.set_visible(!es_monto_);
        btn_peek_.set_icon(LV_SYMBOL_EYE_OPEN);

        set_key_enabled(btn_dot_, es_monto_);
        set_key_enabled(btn_00_,  es_monto_);

        refresh_display();
    }

    void set_titulo(adw::Str titulo)         { lbl_titulo_.set_text(titulo); }
    void set_max_pin_length(size_t len)      { max_pin_len_ = len > 0 ? len : 6; }
    void clear()                             { buffer_.clear(); clear_error_state(); refresh_display(); }

    bool es_modo_monto() const noexcept      { return es_monto_; }
    const std::string& get_pin() const       { return buffer_; }
    double get_monto() const noexcept {
        if (buffer_.empty()) return 0.0;
        return std::strtod(buffer_.c_str(), nullptr);
    }

    // Resalta la caja en rojo cuando el controlador rechaza el NIP o monto
    void mostrar_error(adw::Str mensaje) {
        lv_obj_set_style_border_color(display_box_, ADW_COLOR_ERROR_TEXT, 0);
        lv_obj_set_style_border_opa(display_box_, LV_OPA_COVER, 0);
        lbl_hint_.set_text(mensaje);
        lbl_hint_.set_color(ADW_COLOR_ERROR_TEXT);

        error_timer_.start(1800, [this]() {
            clear_error_state();
            return false;
        });
    }

    // Útil si el preview o alguna subclase quiere agregar un botón extra a la derecha
    adw::Box& top_bar() noexcept { return top_bar_; }

protected:
    // Hooks virtuales idénticos a tu VBaseNip de GTK4
    virtual void on_btn_nip_back() {}
    virtual void on_btn_nip_enter() {}

    adw::Box root_;
    adw::Box container_;
    adw::Box top_bar_;
    adw::IconButton v_btn_nip_back_;

private:
    void build_top_bar() {
        top_bar_ = adw::Box(container_, adw::Orientation::Horizontal, 12);
        top_bar_.set_align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        // Equivalente a btn_nip_back (osd)
        v_btn_nip_back_ = adw::IconButton(top_bar_, LV_SYMBOL_LEFT, adw::BtnVariant::Default);
        v_btn_nip_back_.set_size_request(42, 42);
        v_btn_nip_back_.signal_clicked().connect([this]() { on_btn_nip_back(); });

        lbl_titulo_ = adw::Label(top_bar_, "Acceso con NIP", ADW_FONT_TITLE_3, ADW_COLOR_FG);
        lbl_titulo_.set_flex_grow(1);

        // Píldora discreta de modo ("PIN" / "MXN")
        lv_obj_t* badge = lv_obj_create(top_bar_.raw());
        lv_obj_remove_style_all(badge);
        adw_set_scrollable(badge, false);
        lv_obj_set_size(badge, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_radius(badge, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(badge, ADW_COLOR_ACCENT_TEXT, 0);
        lv_obj_set_style_bg_opa(badge, 30, 0);
        lv_obj_set_style_pad_hor(badge, 12, 0);
        lv_obj_set_style_pad_ver(badge, 4, 0);

        adw::Widget badge_w(badge);
        lbl_mode_badge_ = adw::Label(badge_w, "PIN", ADW_FONT_MONO, ADW_COLOR_ACCENT_TEXT);
    }

    void build_display() {
        // Caja de visualización (reemplaza ety_pin y spn_monto)
        display_box_ = lv_obj_create(container_.raw());
        lv_obj_remove_style_all(display_box_);
        adw_set_scrollable(display_box_, false);
        lv_obj_set_size(display_box_, lv_pct(100), 76);
        lv_obj_set_flex_flow(display_box_, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(display_box_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(display_box_, 16, 0);
        lv_obj_set_style_pad_column(display_box_, 10, 0);
        lv_obj_set_style_radius(display_box_, 14, 0);
        lv_obj_set_style_bg_color(display_box_, ADW_COLOR_VIEW_BG, 0);
        lv_obj_set_style_bg_opa(display_box_, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(display_box_, 2, 0);
        lv_obj_set_style_border_color(display_box_, ADW_COLOR_FG, 0);
        lv_obj_set_style_border_opa(display_box_, 28, 0);

        adw::Widget disp_w(display_box_);
        lbl_value_ = adw::Label(disp_w, "Digite NIP", ADW_FONT_TITLE_1, ADW_COLOR_FG);
        lbl_value_.set_flex_grow(1);
        lv_obj_set_style_text_align(lbl_value_.raw(), LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_long_mode(lbl_value_.raw(), LV_LABEL_LONG_CLIP);

        // Botón ojo (solo visible en modo NIP)
        btn_peek_ = adw::IconButton(disp_w, LV_SYMBOL_EYE_OPEN, adw::BtnVariant::Flat);
        btn_peek_.signal_clicked().connect([this]() {
            peek_pin_ = !peek_pin_;
            btn_peek_.set_icon(peek_pin_ ? LV_SYMBOL_EYE_CLOSE : LV_SYMBOL_EYE_OPEN);
            refresh_display();
        });

        // Línea sutil de ayuda o error debajo de la caja
        lbl_hint_ = adw::Label(container_, "", ADW_FONT_CAPTION, ADW_COLOR_FG);
        lbl_hint_.set_hexpand(true);
        lv_obj_set_style_text_align(lbl_hint_.raw(), LV_TEXT_ALIGN_CENTER, 0);
    }

    lv_obj_t* make_key(adw::Widget& parent, const char* text, bool is_icon,
                       lv_color_t bg, lv_opa_t bg_opa, lv_opa_t act_opa,
                       lv_color_t fg, uint8_t grow, std::function<void()> on_press)
    {
        lv_obj_t* btn = lv_button_create(parent.raw());
        lv_obj_remove_style_all(btn);
        lv_obj_set_size(btn, lv_pct(100), lv_pct(100));
        lv_obj_set_flex_grow(btn, grow);
        lv_obj_set_style_radius(btn, 12, 0);
        lv_obj_set_style_bg_color(btn, bg, 0);
        lv_obj_set_style_bg_opa(btn, bg_opa, 0);
        lv_obj_set_style_bg_opa(btn, act_opa, LV_STATE_PRESSED);
        lv_obj_set_style_opa(btn, ADW_OPA_DIM, LV_STATE_DISABLED);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_border_color(btn, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(btn, 50, 0);

        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, text);
        lv_obj_set_style_text_font(lbl, is_icon ? ADW_FONT_ICONS : ADW_FONT_TITLE_2, 0);
        lv_obj_set_style_text_color(lbl, fg, 0);
        lv_obj_center(lbl);

        adw::detail::connect_raw(btn, LV_EVENT_CLICKED, [fn = std::move(on_press)](lv_event_t*) {
            if (fn) fn();
        });
        return btn;
    }

    static void set_key_enabled(lv_obj_t* btn, bool en) {
        if (!btn) return;
        if (en) {
            lv_obj_remove_state(btn, LV_STATE_DISABLED);
            adw_set_clickable(btn, true);
        } else {
            lv_obj_add_state(btn, LV_STATE_DISABLED);
            adw_set_clickable(btn, false);
        }
    }

    void build_keypad() {
        adw::Box pad(container_, adw::Orientation::Horizontal, 10);
        pad.set_flex_grow(1);

        // ---- Izquierda (3 columnas x 4 filas de dígitos) ----
        adw::Box digits_col(pad, adw::Orientation::Vertical, 8);
        digits_col.set_flex_grow(3);
        digits_col.set_height(lv_pct(100));

        auto add_digit_row = [&](const char* a, const char* b, const char* c) {
            adw::Box row(digits_col, adw::Orientation::Horizontal, 8);
            row.set_flex_grow(1);
            for (const char* k : {a, b, c}) {
                std::string s(k);
                make_key(row, k, false, ADW_COLOR_FG, ADW_OPA_CTRL_BG, ADW_OPA_CTRL_ACT,
                         ADW_COLOR_FG, 1, [this, s]() { on_digit(s); });
            }
        };

        add_digit_row("1", "2", "3");
        add_digit_row("4", "5", "6");
        add_digit_row("7", "8", "9");

        // Fila 4: [ . ]  [ 0 ]  [ 00 ]
        adw::Box row4(digits_col, adw::Orientation::Horizontal, 8);
        row4.set_flex_grow(1);
        btn_dot_ = make_key(row4, ".", false, ADW_COLOR_FG, ADW_OPA_CARD_BG, ADW_OPA_CTRL_ACT,
                            ADW_COLOR_FG, 1, [this]() { on_dot(); });
        make_key(row4, "0", false, ADW_COLOR_FG, ADW_OPA_CTRL_BG, ADW_OPA_CTRL_ACT,
                 ADW_COLOR_FG, 1, [this]() { on_digit("0"); });
        btn_00_ = make_key(row4, "00", false, ADW_COLOR_FG, ADW_OPA_CARD_BG, ADW_OPA_CTRL_ACT,
                           ADW_COLOR_FG, 1, [this]() { on_digit("0"); on_digit("0"); });

        // ---- Derecha (columna de acciones: Borrar, Limpiar, Aceptar x2) ----
        adw::Box actions_col(pad, adw::Orientation::Vertical, 8);
        actions_col.set_flex_grow(1);
        actions_col.set_height(lv_pct(100));

        make_key(actions_col, LV_SYMBOL_BACKSPACE, true,
                 ADW_COLOR_WARNING_TEXT, 32, 65, ADW_COLOR_WARNING_TEXT, 1,
                 [this]() { on_backspace(); });

        make_key(actions_col, "C", false,
                 ADW_COLOR_ERROR_TEXT, 30, 65, ADW_COLOR_ERROR_TEXT, 1,
                 [this]() { clear(); });

        // Botón Aceptar de doble altura
        lv_obj_t* btn_ok = lv_button_create(actions_col.raw());
        lv_obj_remove_style_all(btn_ok);
        lv_obj_set_size(btn_ok, lv_pct(100), lv_pct(100));
        lv_obj_set_flex_grow(btn_ok, 2);
        lv_obj_set_flex_flow(btn_ok, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(btn_ok, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_row(btn_ok, 6, 0);
        lv_obj_set_style_radius(btn_ok, 12, 0);
        lv_obj_set_style_bg_color(btn_ok, ADW_COLOR_ACCENT_BG, 0);
        lv_obj_set_style_bg_opa(btn_ok, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(btn_ok, ADW_COLOR_ACCENT_ACT, LV_STATE_PRESSED);

        adw::Widget ok_w(btn_ok);
        adw::Label ok_ico(ok_w, LV_SYMBOL_OK, ADW_FONT_ICONS, ADW_COLOR_FG);
        adw::Label ok_txt(ok_w, "Aceptar", ADW_FONT_HEADING, ADW_COLOR_FG);
        adw_set_clickable(ok_ico.raw(), false);
        adw_set_clickable(ok_txt.raw(), false);

        adw::detail::connect_raw(btn_ok, LV_EVENT_CLICKED, [this](lv_event_t*) {
            on_btn_nip_enter();
        });
    }

    void on_digit(const std::string& d) {
        clear_error_state();

        if (!es_monto_) {
            if (buffer_.size() >= max_pin_len_) return;
            buffer_ += d;
            refresh_display();
            return;
        }

        // Modo Monto: hasta 6 dígitos enteros (999,999) y 2 decimales
        size_t dot = buffer_.find('.');
        if (dot == std::string::npos) {
            if (buffer_ == "0") {
                if (d == "0") return;
                buffer_ = d;
            } else {
                if (buffer_.size() >= 6) return;
                buffer_ += d;
            }
        } else {
            size_t decimals = buffer_.size() - dot - 1;
            if (decimals >= 2) return;
            buffer_ += d;
        }
        refresh_display();
    }

    void on_dot() {
        if (!es_monto_) return;
        clear_error_state();
        if (buffer_.find('.') != std::string::npos) return;
        if (buffer_.empty()) buffer_ = "0.";
        else                 buffer_.push_back('.');
        refresh_display();
    }

    void on_backspace() {
        clear_error_state();
        if (!buffer_.empty()) {
            buffer_.pop_back();
            refresh_display();
        }
    }

    void clear_error_state() {
        lv_obj_set_style_border_color(display_box_, ADW_COLOR_FG, 0);
        lv_obj_set_style_border_opa(display_box_, 28, 0);
        lbl_hint_.set_color(ADW_COLOR_FG, ADW_OPA_DIM);
        lbl_hint_.set_text(
            es_monto_
                ? "Máximo $999,999.99 MXN · Hasta 2 decimales"
                : "Ingrese su clave de acceso y pulse Aceptar");
    }

    static std::string format_amount_display(const std::string& raw) {
        size_t dot = raw.find('.');
        std::string int_part  = (dot == std::string::npos) ? raw : raw.substr(0, dot);
        std::string frac_part = (dot == std::string::npos) ? ""  : raw.substr(dot);

        // Separador de miles
        std::string grouped;
        for (size_t i = 0; i < int_part.size(); ++i) {
            if (i > 0 && ((int_part.size() - i) % 3 == 0)) grouped.push_back(',');
            grouped.push_back(int_part[i]);
        }
        return "$ " + grouped + frac_part + " MXN";
    }

    void refresh_display() {
        if (!es_monto_) {
            if (buffer_.empty()) {
                lbl_value_.set_font(ADW_FONT_TITLE_2);
                lbl_value_.set_color(ADW_COLOR_FG, ADW_OPA_DIM);
                lbl_value_.set_text("Digite NIP");
            } else if (peek_pin_) {
                std::string spaced;
                for (size_t i = 0; i < buffer_.size(); ++i) {
                    if (i > 0) spaced += "  ";
                    spaced.push_back(buffer_[i]);
                }
                lbl_value_.set_font(ADW_FONT_MONO_LG);
                lbl_value_.set_color(ADW_COLOR_ACCENT_TEXT, LV_OPA_COVER);
                lbl_value_.set_text(spaced);
            } else {
                std::string dots;
                for (size_t i = 0; i < buffer_.size(); ++i) {
                    if (i > 0) dots += "  ";
                    dots += "\xE2\x80\xA2"; // '•' (0x2022)
                }
                lbl_value_.set_font(ADW_FONT_TITLE_1);
                lbl_value_.set_color(ADW_COLOR_FG, LV_OPA_COVER);
                lbl_value_.set_text(dots);
            }
            return;
        }

        // Modo Monto
        if (buffer_.empty()) {
            lbl_value_.set_font(ADW_FONT_MONO_LG);
            lbl_value_.set_color(ADW_COLOR_FG, ADW_OPA_DIM);
            lbl_value_.set_text("$ 0.00 MXN");
        } else {
            lbl_value_.set_font(ADW_FONT_MONO_LG);
            lbl_value_.set_color(ADW_COLOR_SUCCESS_TEXT, LV_OPA_COVER);
            lbl_value_.set_text(format_amount_display(buffer_));
        }
    }

    bool es_monto_      = false;
    bool peek_pin_      = false;
    size_t max_pin_len_ = 6;
    std::string buffer_;

    adw::Label lbl_titulo_;
    adw::Label lbl_mode_badge_;
    lv_obj_t* display_box_ = nullptr;
    adw::Label lbl_value_;
    adw::IconButton btn_peek_;
    adw::Label lbl_hint_;

    lv_obj_t* btn_dot_ = nullptr;
    lv_obj_t* btn_00_  = nullptr;
    adw::Timer error_timer_;
};

// ============================================================================
// SIMULADOR INTERACTIVO PARA LVGL LIVE PREVIEW
// ============================================================================
class NipControllerDemo : public VBaseNip {
public:
    explicit NipControllerDemo(adw::Widget& parent)
        : VBaseNip(parent)
    {
        // Botón extra solo en el preview para alternar entre Modo NIP y Modo Monto
        btn_toggle_mode_ = adw::Button(top_bar(), "Modo Monto", adw::BtnVariant::Default);
        btn_toggle_mode_.signal_clicked().connect([this]() {
            bool nuevo_modo = !es_modo_monto();
            set_modo_monto(nuevo_modo);
            btn_toggle_mode_.set_label(nuevo_modo ? "Modo NIP" : "Modo Monto");
        });
    }

protected:
    void on_btn_nip_back() override {
        adw::Toast::show("Regresando a la pantalla anterior...", 1600);
    }

    void on_btn_nip_enter() override {
        if (!es_modo_monto()) {
            if (get_pin().size() < 4) {
                mostrar_error("El NIP debe tener al menos 4 dígitos");
                return;
            }
            if (get_pin() == "1234") {
                adw::Toast::show("NIP correcto — acceso concedido", 2000);
                clear();
            } else {
                mostrar_error("NIP incorrecto (prueba 1234)");
                clear();
            }
            return;
        }

        double monto = get_monto();
        if (monto <= 0.0) {
            mostrar_error("Ingrese un monto mayor a $0.00");
            return;
        }
        char msg[64];
        std::snprintf(msg, sizeof(msg), "Monto confirmado: $%.2f MXN", monto);
        adw::Toast::show(msg, 2200);
    }

private:
    adw::Button btn_toggle_mode_;
};

struct NipPreviewHost {
    adw::Screen scr;
    NipControllerDemo vista;

    NipPreviewHost()
        : scr(lv_screen_active())
        , vista(scr)
    {}
};

static std::unique_ptr<NipPreviewHost> g_nip_app;

static void ui_init(void) {
    adw::UiDispatcher::init();
    g_nip_app.reset();
    g_nip_app = std::make_unique<NipPreviewHost>();
}

#ifdef LVGL_LIVE_PREVIEW
extern "C" void lvgl_live_preview_init(void) {
    ui_init();
}
#endif
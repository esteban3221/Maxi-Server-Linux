#include "adw_lvgl.h"

#include <cstdio>
#include <memory>
#include <string>

static std::string fmt_mxn(double val) {
    char buf[40];
    std::snprintf(buf, sizeof(buf), "$%.2f MXN", val < 0.0 ? 0.0 : val);
    return buf;
}

// ============================================================================
// VISTA MÉTODO DE PAGO (equivalente directo a src/view/metodo_pago.cpp)
// ============================================================================
class VMetodoPago {
public:
    explicit VMetodoPago(adw::Widget& parent)
        : root_(parent, adw::Orientation::Vertical, 20)
    {
        root_.set_size_request(lv_pct(100), lv_pct(100));
        root_.set_flex_grow(1);
        root_.set_padding(20);
        root_.set_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        build_header();
        build_payment_grid();
    }

    // API para el controlador
    void set_monto(double total, bool visible = true) {
        adw_set_hidden(amount_pill_, !visible);
        if (visible) {
            lbl_monto_.set_text("Total a pagar: " + fmt_mxn(total));
        }
    }

    void set_cash_enabled(bool en)     { v_btn_cash_.set_sensitive(en); }
    void set_card_enabled(bool en)     { v_btn_card_.set_sensitive(en); }
    void set_deferred_enabled(bool en) { v_btn_deferred_.set_sensitive(en); }
    void set_deferred_visible(bool v)  { v_btn_deferred_.set_visible(v); }

    // Señales equivalentes a v_btn_*->signal_clicked()
    adw::SignalProxy<> signal_cash_clicked()     { return v_btn_cash_.signal_clicked(); }
    adw::SignalProxy<> signal_card_clicked()     { return v_btn_card_.signal_clicked(); }
    adw::SignalProxy<> signal_deferred_clicked() { return v_btn_deferred_.signal_clicked(); }
    adw::SignalProxy<> signal_cancel_clicked()   { return v_btn_cancela_.signal_clicked(); }

private:
    static constexpr int32_t kCardW   = 250;
    static constexpr int32_t kCardH   = 176;
    static constexpr int32_t kGridGap = 14;
    static constexpr int32_t kGridW   = (kCardW * 2) + kGridGap; // 514 px

    void build_header() {
        adw::Box hdr(root_, adw::Orientation::Horizontal, 16);
        hdr.set_width(kGridW);
        hdr.set_align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        // Botón circular rojo destructivo (igual a circular + destructive-action)
        v_btn_cancela_ = adw::IconButton(hdr, LV_SYMBOL_CLOSE, adw::BtnVariant::Destructive);
        v_btn_cancela_.set_size_request(44, 44);
        lv_obj_set_style_radius(v_btn_cancela_.raw(), LV_RADIUS_CIRCLE, 0);

        adw::Box title_col(hdr, adw::Orientation::Vertical, 4);
        title_col.set_flex_grow(1);

        adw::Label lbl_title(title_col, "Seleccione el método de pago", ADW_FONT_TITLE_2, ADW_COLOR_FG);

        // Píldora discreta con el monto total (opcional)
        amount_pill_ = lv_obj_create(title_col.raw());
        lv_obj_remove_style_all(amount_pill_);
        adw_set_scrollable(amount_pill_, false);
        lv_obj_set_size(amount_pill_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_radius(amount_pill_, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(amount_pill_, ADW_COLOR_ACCENT_TEXT, 0);
        lv_obj_set_style_bg_opa(amount_pill_, 28, 0);
        lv_obj_set_style_pad_hor(amount_pill_, 12, 0);
        lv_obj_set_style_pad_ver(amount_pill_, 4, 0);

        adw::Widget pill_w(amount_pill_);
        lbl_monto_ = adw::Label(pill_w, "Total a pagar: $0.00 MXN", ADW_FONT_MONO, ADW_COLOR_ACCENT_TEXT);
    }

    void build_payment_grid() {
        adw::Box grid(root_, adw::Orientation::Vertical, kGridGap);
        grid.set_width(kGridW);
        grid.set_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        // ---- Fila 0: Efectivo | Tarjeta ----
        adw::Box row_top(grid, adw::Orientation::Horizontal, kGridGap);
        row_top.set_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        v_btn_cash_ = adw::ActionCard(
            row_top, "$", ADW_COLOR_SUCCESS_TEXT,
            "Efectivo", "Billetes y monedas\ncon cambio automático",
            kCardW, kCardH);
        // Sube el tamaño del símbolo "$" usando Ubuntu Bold 28
        if (lv_obj_t* badge = lv_obj_get_child(v_btn_cash_.raw(), 0)) {
            if (lv_obj_t* ico = lv_obj_get_child(badge, 0)) {
                lv_obj_set_style_text_font(ico, ADW_FONT_TITLE_1, 0);
            }
        }

        v_btn_card_ = adw::ActionCard(
            row_top, LV_SYMBOL_WIFI, ADW_COLOR_ACCENT_TEXT,
            "Tarjeta", "Débito, crédito o\npago sin contacto",
            kCardW, kCardH);

        // ---- Fila 1 (column-span: 2): Diferir Pago ----
        v_btn_deferred_ = create_wide_card(
            grid, LV_SYMBOL_SHUFFLE, ADW_COLOR_WARNING_TEXT,
            "Diferir Pago", "Combinar efectivo y tarjeta en una misma operación",
            kGridW, 84);
    }

    adw::Widget create_wide_card(adw::Widget& parent, const char* symbol, lv_color_t icon_color,
                                 const char* titulo, const char* subtitulo,
                                 int32_t w, int32_t h)
    {
        lv_obj_t* card = lv_button_create(parent.raw());
        lv_obj_remove_style_all(card);
        lv_obj_set_size(card, w, h);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(card, 20, 0);
        lv_obj_set_style_pad_ver(card, 14, 0);
        lv_obj_set_style_pad_column(card, 16, 0);
        lv_obj_set_style_radius(card, 14, 0);
        lv_obj_set_style_bg_color(card, ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(card, ADW_OPA_CARD_BG, 0);
        lv_obj_set_style_bg_opa(card, ADW_OPA_CARD_ACT, LV_STATE_PRESSED);
        lv_obj_set_style_border_width(card, 1, 0);
        lv_obj_set_style_border_color(card, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(card, 50, 0);
        lv_obj_set_style_shadow_width(card, 12, 0);
        lv_obj_set_style_shadow_offset_y(card, 2, 0);
        lv_obj_set_style_shadow_color(card, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_shadow_opa(card, 55, 0);

        // Insignia circular izquierda
        lv_obj_t* badge = lv_obj_create(card);
        lv_obj_remove_style_all(badge);
        adw_set_clickable(badge, false);
        lv_obj_set_size(badge, 48, 48);
        lv_obj_set_style_radius(badge, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(badge, icon_color, 0);
        lv_obj_set_style_bg_opa(badge, 38, 0);

        lv_obj_t* ico = lv_label_create(badge);
        lv_label_set_text(ico, symbol);
        lv_obj_set_style_text_font(ico, ADW_FONT_ICONS, 0);
        lv_obj_set_style_text_color(ico, icon_color, 0);
        lv_obj_center(ico);

        // Textos centrales
        lv_obj_t* text_col = lv_obj_create(card);
        lv_obj_remove_style_all(text_col);
        adw_set_scrollable(text_col, false);
        adw_set_clickable(text_col, false);
        lv_obj_set_height(text_col, LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(text_col, 1);
        lv_obj_set_flex_flow(text_col, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(text_col, 2, 0);

        lv_obj_t* lbl_t = lv_label_create(text_col);
        lv_label_set_text(lbl_t, titulo);
        lv_obj_set_style_text_font(lbl_t, ADW_FONT_HEADING, 0);
        lv_obj_set_style_text_color(lbl_t, ADW_COLOR_FG, 0);

        lv_obj_t* lbl_s = lv_label_create(text_col);
        lv_label_set_text(lbl_s, subtitulo);
        lv_obj_set_style_text_font(lbl_s, ADW_FONT_CAPTION, 0);
        lv_obj_set_style_text_color(lbl_s, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_opa(lbl_s, ADW_OPA_DIM, 0);

        // Flecha derecha
        lv_obj_t* arrow = lv_label_create(card);
        lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
        lv_obj_set_style_text_font(arrow, ADW_FONT_ICONS, 0);
        lv_obj_set_style_text_color(arrow, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_opa(arrow, ADW_OPA_DIM, 0);

        return adw::Widget(card);
    }

    adw::Box root_;
    adw::IconButton v_btn_cancela_;
    lv_obj_t* amount_pill_ = nullptr;
    adw::Label lbl_monto_;

    adw::ActionCard v_btn_cash_;
    adw::ActionCard v_btn_card_;
    adw::Widget v_btn_deferred_;
};

// ============================================================================
// SIMULADOR INTERACTIVO PARA LVGL LIVE PREVIEW
// ============================================================================
class MetodoPagoPreviewHost {
public:
    MetodoPagoPreviewHost()
        : scr_(lv_screen_active())
        , vista_(scr_)
    {
        vista_.set_monto(245.50, true);

        vista_.signal_cash_clicked().connect([]() {
            adw::Toast::show("Iniciando cobro en Efectivo...", 1800);
        });

        vista_.signal_card_clicked().connect([]() {
            adw::Toast::show("Conectando con terminal de Tarjeta...", 1800);
        });

        vista_.signal_deferred_clicked().connect([]() {
            adw::Toast::show("Abriendo flujo de Pago Diferido...", 1800);
        });

        vista_.signal_cancel_clicked().connect([]() {
            adw::AlertDialog::show(
                "¿Cancelar operación?",
                "Se cancelará la selección del método de pago y volverá al inicio.",
                "Continuar",
                "Cancelar",
                adw::BtnVariant::Destructive,
                []() {
                    adw::Toast::show("Operación cancelada", 1800);
                });
        });
    }

private:
    adw::Screen scr_;
    VMetodoPago vista_;
};

static std::unique_ptr<MetodoPagoPreviewHost> g_metodo_pago_app;

static void ui_init(void) {
    adw::UiDispatcher::init();
    g_metodo_pago_app.reset();
    g_metodo_pago_app = std::make_unique<MetodoPagoPreviewHost>();
}

#ifdef LVGL_LIVE_PREVIEW
extern "C" void lvgl_live_preview_init(void) {
    ui_init();
}
#endif
#include "adw_lvgl.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

struct MLevelCashMock {
    double denominacion = 0.0;
    int cant_alm        = 0;
    int cant_recy       = 0;
    int ingreso         = 0;
    int nivel_inmo      = 0;
};

static std::string fmt_mxn(double val) {
    char buf[40];
    std::snprintf(buf, sizeof(buf), "$%.2f MXN", val);
    return buf;
}

static std::string fmt_denom(double val) {
    char buf[24];
    if (val < 1.0) std::snprintf(buf, sizeof(buf), "$%.2f", val);
    else           std::snprintf(buf, sizeof(buf), "$%.0f", val);
    return buf;
}

class VRefillPreview {
public:
    VRefillPreview()
        : scr_(lv_screen_active())
        , header_(scr_, "Recarga de Efectivo (Refill)", false)
        , split_(scr_, adw::Orientation::Horizontal, 14)
    {
        // Botón temporal en cabecera para probar ingresos en vivo en el preview
        btn_sim_ = adw::Button(header_, "+ Simular", adw::BtnVariant::Suggested);
        header_.pack_end(btn_sim_);

        split_.set_flex_grow(1);
        split_.set_padding(14);

        init_mock_data();
        build_left_tables();
        build_right_summary();
        connect_signals();

        render_all();
        start_auto_demo();
    }

private:
    void init_mock_data() {
        monedas_ = {
            {0.50,  18, 40, 0, 20},
            {1.00,  32, 65, 0, 30},
            {2.00,  24, 50, 0, 25},
            {5.00,  15, 45, 0, 20},
            {10.00, 10, 35, 0, 15},
        };
        billetes_ = {
            {20.0,  12, 25, 0, 10},
            {50.0,   8, 20, 0, 10},
            {100.0,  6, 18, 0,  8},
            {200.0,  4, 10, 0,  5},
            {500.0,  2,  4, 0,  2},
        };
    }

    adw::Box create_card(adw::Widget& parent, bool accent_border = false) {
        adw::Box card(parent, adw::Orientation::Vertical, 8);
        card.set_padding(12);
        lv_obj_set_style_bg_color(card.raw(), ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(card.raw(), ADW_OPA_CARD_BG, 0);
        lv_obj_set_style_radius(card.raw(), 12, 0);
        lv_obj_set_style_border_width(card.raw(), 1, 0);
        lv_obj_set_style_border_color(card.raw(),
            accent_border ? ADW_COLOR_ACCENT_TEXT : ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(card.raw(), accent_border ? 110 : 45, 0);
        return card;
    }

    adw::Label create_pill_badge(adw::Widget& parent, lv_color_t color) {
        lv_obj_t* pill = lv_obj_create(parent.raw());
        lv_obj_remove_style_all(pill);
        adw_set_scrollable(pill, false);
        lv_obj_set_size(pill, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_radius(pill, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(pill, color, 0);
        lv_obj_set_style_bg_opa(pill, 34, 0);
        lv_obj_set_style_pad_hor(pill, 10, 0);
        lv_obj_set_style_pad_ver(pill, 3, 0);

        adw::Widget pill_w(pill);
        adw::Label lbl(pill_w, "$0.00 MXN", ADW_FONT_MONO, color);
        return lbl;
    }

    void build_left_tables() {
        adw::Box left_col(split_, adw::Orientation::Vertical, 12);
        left_col.set_flex_grow(1);
        lv_obj_set_height(left_col.raw(), lv_pct(100));

        const std::vector<adw::ColumnSpec> cols = {
            {"Denom.",  2, LV_TEXT_ALIGN_LEFT},
            {"Alm.",    1, LV_TEXT_ALIGN_RIGHT},
            {"Recy.",   1, LV_TEXT_ALIGN_RIGHT},
            {"Ingreso", 2, LV_TEXT_ALIGN_RIGHT},
            {"Inmo.",   1, LV_TEXT_ALIGN_RIGHT},
        };

        // ---- Tarjeta Monedas (50% alto) ----
        auto card_mon = create_card(left_col);
        card_mon.set_flex_grow(1);

        adw::Box hdr_mon(card_mon, adw::Orientation::Horizontal, 8);
        hdr_mon.set_align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        adw::Label title_mon(hdr_mon, "Monedas", ADW_FONT_HEADING);
        title_mon.set_flex_grow(1);
        v_lbl_total_parcial_monedas_ = create_pill_badge(hdr_mon, ADW_COLOR_SUCCESS_TEXT);

        v_tree_reciclador_monedas_ = adw::ColumnView(card_mon, cols);
        for (const auto& m : monedas_) {
            v_tree_reciclador_monedas_.add_row({
                fmt_denom(m.denominacion),
                std::to_string(m.cant_alm),
                std::to_string(m.cant_recy),
                "0",
                std::to_string(m.nivel_inmo)
            });
        }

        // ---- Tarjeta Billetes (50% alto) ----
        auto card_bil = create_card(left_col);
        card_bil.set_flex_grow(1);

        adw::Box hdr_bil(card_bil, adw::Orientation::Horizontal, 8);
        hdr_bil.set_align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        adw::Label title_bil(hdr_bil, "Billetes", ADW_FONT_HEADING);
        title_bil.set_flex_grow(1);
        v_lbl_total_parcial_billetes_ = create_pill_badge(hdr_bil, ADW_COLOR_SUCCESS_TEXT);

        v_tree_reciclador_billetes_ = adw::ColumnView(card_bil, cols);
        for (const auto& b : billetes_) {
            v_tree_reciclador_billetes_.add_row({
                fmt_denom(b.denominacion),
                std::to_string(b.cant_alm),
                std::to_string(b.cant_recy),
                "0",
                std::to_string(b.nivel_inmo)
            });
        }
    }

    adw::Label build_kpi_card(adw::Widget& parent, const char* title,
                              lv_color_t value_color, const lv_font_t* font,
                              bool accent_border = false) {
        auto card = create_card(parent, accent_border);
        adw::Label lbl_t(card, title, ADW_FONT_CAPTION);
        lbl_t.set_color(ADW_COLOR_FG, ADW_OPA_DIM);

        adw::Label lbl_v(card, "$0.00 MXN", font, value_color);
        return lbl_v;
    }

    void build_right_summary() {
        adw::Box right_col(split_, adw::Orientation::Vertical, 12);
        right_col.set_size_request(240, lv_pct(100));

        // Indicador de estado del validador
        adw::Box status_box(right_col, adw::Orientation::Horizontal, 8);
        status_box.set_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        status_box.set_padding(8);
        lv_obj_set_style_radius(status_box.raw(), 10, 0);
        lv_obj_set_style_bg_color(status_box.raw(), ADW_COLOR_SUCCESS_TEXT, 0);
        lv_obj_set_style_bg_opa(status_box.raw(), 26, 0);
        status_pill_ = status_box.raw();
        lbl_status_ = adw::Label(status_box, "Aceptando efectivo",
                                 ADW_FONT_CAPTION, ADW_COLOR_SUCCESS_TEXT);

        // 3 tarjetas KPI exactas de tu vista original
        v_lbl_total_ = build_kpi_card(
            right_col, "Total General", ADW_COLOR_ACCENT_TEXT, ADW_FONT_MONO_LG, true);
        v_lbl_total_monedas_ = build_kpi_card(
            right_col, "Total Monedas", ADW_COLOR_FG, ADW_FONT_MONO);
        v_lbl_total_billetes_ = build_kpi_card(
            right_col, "Total Billetes", ADW_COLOR_FG, ADW_FONT_MONO);

        // Espaciador flexible para anclar el botón Detener abajo
        adw::Box spacer(right_col);
        spacer.set_flex_grow(1);

        v_btn_deten_ = adw::Button(right_col, "Detener", adw::BtnVariant::Destructive, false);
        v_btn_deten_.set_hexpand(true);
        v_btn_deten_.set_height(48);
    }

    void connect_signals() {
        btn_sim_.signal_clicked().connect([this]() {
            if (!activo_) {
                activo_ = true;
                update_status_badge();
                start_auto_demo();
                adw::Toast::show("Recarga reanudada", 1500);
                return;
            }
            simular_ingreso();
        });

        v_btn_deten_.signal_clicked().connect([this]() {
            if (!activo_) return;
            adw::AlertDialog::show(
                "¿Detener recarga?",
                "Se cerrará la sesión de refill y se guardará el ingreso acumulado.",
                "Continuar",
                "Detener",
                adw::BtnVariant::Destructive,
                [this]() {
                    activo_ = false;
                    sim_timer_.stop();
                    update_status_badge();
                    adw::Toast::show("Recarga finalizada correctamente");
                });
        });
    }

    void update_status_badge() {
        lv_color_t col = activo_ ? ADW_COLOR_SUCCESS_TEXT : ADW_COLOR_WARNING_TEXT;
        lv_obj_set_style_bg_color(status_pill_, col, 0);
        lbl_status_.set_color(col);
        lbl_status_.set_text(activo_ ? LV_SYMBOL_OK "  Aceptando efectivo"
                                     : LV_SYMBOL_PAUSE "  Recarga detenida");
        v_btn_deten_.set_sensitive(activo_);
        btn_sim_.set_label(activo_ ? "+ Simular" : "Reanudar");
    }

    void start_auto_demo() {
        sim_timer_.start(1400, [this]() {
            if (!activo_) return false;
            simular_ingreso();
            return true;
        });
    }

    void simular_ingreso() {
        static constexpr size_t kSteps[] = {3, 4, 10, 11, 2, 12, 4, 10};
        size_t code = kSteps[step_++ % (sizeof(kSteps) / sizeof(kSteps[0]))];

        v_tree_reciclador_monedas_.clear_highlights();
        v_tree_reciclador_billetes_.clear_highlights();

        if (code < 10) {
            auto& m = monedas_[code];
            m.ingreso++;
            m.cant_recy++;
            update_table_row(v_tree_reciclador_monedas_, code, m);
            v_tree_reciclador_monedas_.set_row_highlight(code, true);
        } else {
            size_t idx = code - 10;
            auto& b = billetes_[idx];
            b.ingreso++;
            b.cant_recy++;
            update_table_row(v_tree_reciclador_billetes_, idx, b);
            v_tree_reciclador_billetes_.set_row_highlight(idx, true);
        }
        recalc_totals();
    }

    void update_table_row(adw::ColumnView& table, size_t idx, const MLevelCashMock& item) {
        table.set_cell(idx, 1, std::to_string(item.cant_alm));
        table.set_cell(idx, 2, std::to_string(item.cant_recy));
        if (item.ingreso > 0) {
            table.set_cell(idx, 3, "+" + std::to_string(item.ingreso), ADW_COLOR_SUCCESS_TEXT);
        } else {
            table.set_cell(idx, 3, "0", ADW_COLOR_FG, ADW_OPA_DIM);
        }
        table.set_cell(idx, 4, std::to_string(item.nivel_inmo), ADW_COLOR_FG, ADW_OPA_DIM);
    }

    void render_all() {
        for (size_t i = 0; i < monedas_.size(); ++i) {
            update_table_row(v_tree_reciclador_monedas_, i, monedas_[i]);
        }
        for (size_t i = 0; i < billetes_.size(); ++i) {
            update_table_row(v_tree_reciclador_billetes_, i, billetes_[i]);
        }
        recalc_totals();
    }

    void recalc_totals() {
        double parcial_mon = 0.0, total_mon = 0.0;
        for (const auto& m : monedas_) {
            parcial_mon += m.denominacion * m.ingreso;
            total_mon   += m.denominacion * (m.cant_alm + m.cant_recy);
        }

        double parcial_bil = 0.0, total_bil = 0.0;
        for (const auto& b : billetes_) {
            parcial_bil += b.denominacion * b.ingreso;
            total_bil   += b.denominacion * (b.cant_alm + b.cant_recy);
        }

        v_lbl_total_parcial_monedas_.set_text("+" + fmt_mxn(parcial_mon));
        v_lbl_total_parcial_billetes_.set_text("+" + fmt_mxn(parcial_bil));
        v_lbl_total_monedas_.set_text(fmt_mxn(total_mon));
        v_lbl_total_billetes_.set_text(fmt_mxn(total_bil));
        v_lbl_total_.set_text(fmt_mxn(total_mon + total_bil));
    }

    adw::Screen scr_;
    adw::HeaderBar header_;
    adw::Button btn_sim_;
    adw::Box split_;

    adw::ColumnView v_tree_reciclador_monedas_;
    adw::ColumnView v_tree_reciclador_billetes_;
    adw::Label v_lbl_total_parcial_monedas_;
    adw::Label v_lbl_total_parcial_billetes_;

    lv_obj_t* status_pill_ = nullptr;
    adw::Label lbl_status_;
    adw::Label v_lbl_total_;
    adw::Label v_lbl_total_monedas_;
    adw::Label v_lbl_total_billetes_;
    adw::Button v_btn_deten_;

    std::vector<MLevelCashMock> monedas_;
    std::vector<MLevelCashMock> billetes_;
    adw::Timer sim_timer_;
    size_t step_ = 0;
    bool activo_ = true;
};

static std::unique_ptr<VRefillPreview> g_refill_app;

static void ui_init(void) {
    adw::UiDispatcher::init();
    g_refill_app.reset();
    g_refill_app = std::make_unique<VRefillPreview>();
}

#ifdef LVGL_LIVE_PREVIEW
extern "C" void lvgl_live_preview_init(void) {
    ui_init();
}
#endif
#include "adw_lvgl.h"

#include <algorithm>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifndef LVGL_LIVE_PREVIEW
#include <filesystem>
#endif

// ============================================================================
// MODELO DESACOPLADO (se llena desde crow::json::rvalue en Linux o ESP-IDF)
// ============================================================================
struct ValidatorInfo {
    std::string device_model     = "Sin Definir";
    std::string device_id        = "--";
    bool        is_open          = false;
    int         ssp_version      = 0;
    std::string device_error     = "Sin conexión";
    std::string firmware         = "--";
    std::string dataset          = "--";
    std::string serial           = "--";
    std::string module_serial    = "--";
    std::string revision         = "--";
    std::string module_revision  = "--";
    std::string real_time_clock  = "--";
};

enum class ValidatorKind {
    Bill,
    Coin
};

// ============================================================================
// COMPONENTE REUTILIZABLE VValidador (reemplaza view/config/base.validador.cpp)
// ============================================================================
class VValidador {
public:
    VValidador(adw::Widget& parent, ValidatorKind kind, uint8_t ssp_address)
        : kind_(kind)
        , ssp_address_(ssp_address)
        , split_(parent, adw::Orientation::Horizontal, 14)
    {
        split_.set_flex_grow(1);
        split_.set_height(lv_pct(100));
        split_.set_padding(14);

        build_left_column();
        build_right_column();
    }

    void set_id_conf(uint16_t id) noexcept { id_conf_ = id; }
    uint16_t get_id_conf() const noexcept  { return id_conf_; }

    // Opcional: permite reemplazar el ícono por una imagen ("A:/assets/nv200.png")
    void set_img(adw::Str lv_img_src) {
        if (lv_img_src.empty() || !badge_icon_) return;
        lv_obj_clean(badge_icon_);
        lv_obj_t* img = lv_image_create(badge_icon_);
        lv_image_set_src(img, lv_img_src);
        lv_obj_set_size(img, 42, 42);
        lv_image_set_inner_align(img, LV_IMAGE_ALIGN_CONTAIN);
        lv_obj_center(img);
        lbl_icon_ = adw::Label();
    }

    void set_ports(const std::vector<std::string>& nuevos_puertos, const std::string& puerto_guardado = "") {
        puertos_ = nuevos_puertos;
        if (!dd_puerto_) return;

        if (puertos_.empty()) {
            lv_dropdown_set_options(dd_puerto_, "Sin puertos /dev/ttyUSB*");
            lv_dropdown_set_selected(dd_puerto_, 0);
            return;
        }

        std::string joined;
        uint32_t match_idx = 0;
        bool found = false;

        for (size_t i = 0; i < puertos_.size(); ++i) {
            if (i > 0) joined.push_back('\n');
            joined += puertos_[i];
            if (!puerto_guardado.empty() && puertos_[i] == puerto_guardado) {
                match_idx = static_cast<uint32_t>(i);
                found = true;
            }
        }

        lv_dropdown_set_options(dd_puerto_, joined.c_str());
        lv_dropdown_set_selected(dd_puerto_, found ? match_idx : 0);
    }

    std::string get_puerto_seleccionado() const {
        if (!dd_puerto_ || puertos_.empty()) return "";
        uint32_t idx = lv_dropdown_get_selected(dd_puerto_);
        return idx < puertos_.size() ? puertos_[idx] : "";
    }

    void set_data(const ValidatorInfo& info) {
        const bool ok = info.is_open && info.device_model != "UNKNOWN";
        const bool has_error = !info.device_error.empty() &&
                               info.device_error != "NONE" &&
                               info.device_error != "OK" &&
                               info.device_error != "0" &&
                               info.device_error != "Sin errores";

        v_lbl_titulo_.set_text(info.device_model.empty() ? "Sin Definir" : info.device_model);

        // Estado de conexión (antes v_lbl_data[1])
        lv_color_t st_col = ok ? ADW_COLOR_SUCCESS_TEXT : ADW_COLOR_ERROR_TEXT;
        lv_obj_set_style_bg_color(status_pill_, st_col, 0);
        lv_obj_set_style_bg_color(status_dot_, st_col, 0);
        lbl_status_.set_color(st_col);
        lbl_status_.set_text(ok ? "Conectado" : "Desconectado");

        // Franja de diagnóstico / error (antes v_lbl_data[3])
        lv_color_t err_col = (!ok || has_error) ? ADW_COLOR_ERROR_TEXT : ADW_COLOR_SUCCESS_TEXT;
        lv_obj_set_style_bg_color(error_box_, err_col, 0);
        lv_obj_set_style_border_color(error_box_, err_col, 0);
        lbl_err_ico_.set_color(err_col);
        lbl_err_ico_.set_text((!ok || has_error) ? LV_SYMBOL_WARNING : LV_SYMBOL_OK);
        lbl_err_txt_.set_color(err_col);
        if (!ok && !has_error) {
            lbl_err_txt_.set_text("Dispositivo no responde");
        } else {
            lbl_err_txt_.set_text(has_error ? info.device_error : "Operación normal (sin errores)");
        }

        // Campos técnicos (Protocolo y Estado)
        row_id_.set_value(non_empty(info.device_id));
        row_ssp_.set_value(info.ssp_version > 0 ? "v" + std::to_string(info.ssp_version) : "--");
        row_fw_.set_value(non_empty(info.firmware));
        row_dataset_.set_value(non_empty(info.dataset));
        row_rtc_.set_value(non_empty(info.real_time_clock));

        // Campos técnicos (Hardware y Series)
        row_serial_.set_value(non_empty(info.serial));
        row_rev_.set_value(non_empty(info.revision));
        row_mod_serial_.set_value(non_empty(info.module_serial));
        row_mod_rev_.set_value(non_empty(info.module_revision));
    }

    adw::SignalProxy<> signal_refresh_ports() {
        return btn_refresh_ports_.signal_clicked();
    }

    static std::vector<std::string> scan_serial_ports() {
        std::vector<std::string> found;
#ifndef LVGL_LIVE_PREVIEW
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator("/dev", ec)) {
            if (!entry.is_character_file(ec)) continue;
            std::string p = entry.path().string();
            if (p.find("ttyUSB") != std::string::npos || p.find("ttyACM") != std::string::npos) {
                found.push_back(std::move(p));
            }
        }
        std::sort(found.begin(), found.end());
#else
        found = {"/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2", "/dev/ttyUSB3"};
#endif
        return found;
    }

private:
    static std::string non_empty(const std::string& s) {
        return s.empty() ? "--" : s;
    }

    static adw::Box create_card(adw::Widget& parent, int32_t pad = 12, int32_t spacing = 10) {
        adw::Box card(parent, adw::Orientation::Vertical, spacing);
        card.set_padding(pad);
        lv_obj_set_style_bg_color(card.raw(), ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(card.raw(), ADW_OPA_CARD_BG, 0);
        lv_obj_set_style_radius(card.raw(), 12, 0);
        lv_obj_set_style_border_width(card.raw(), 1, 0);
        lv_obj_set_style_border_color(card.raw(), ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(card.raw(), 45, 0);
        return card;
    }

    static adw::ValueRow make_compact_row(adw::BoxedList& list, adw::Str titulo, bool divisor = true) {
        adw::ValueRow row(list, titulo, "", "--", ADW_COLOR_FG, ADW_FONT_MONO, divisor);
        if (lv_obj_t* raw_row = lv_obj_get_parent(row.raw())) {
            lv_obj_set_style_min_height(raw_row, 42, 0);
            lv_obj_set_style_pad_ver(raw_row, 6, 0);
            lv_obj_set_style_pad_hor(raw_row, 14, 0);
        }
        lv_label_set_long_mode(row.raw(), LV_LABEL_LONG_DOT);
        lv_obj_set_style_max_width(row.raw(), 240, 0);
        return row;
    }

    void build_left_column() {
        adw::Box left(split_, adw::Orientation::Vertical, 12);
        left.set_size_request(255, lv_pct(100));

        // ---- 1. Tarjeta de identidad y salud ----
        auto hero = create_card(left, 14, 12);

        adw::Box top_row(hero, adw::Orientation::Horizontal, 12);
        top_row.set_align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        // Ícono compacto (48x48 en vez de 250x250)
        lv_color_t kind_col = (kind_ == ValidatorKind::Bill)
            ? ADW_COLOR_SUCCESS_TEXT
            : ADW_COLOR_ACCENT_TEXT;

        badge_icon_ = lv_obj_create(top_row.raw());
        lv_obj_remove_style_all(badge_icon_);
        adw_set_scrollable(badge_icon_, false);
        lv_obj_set_size(badge_icon_, 48, 48);
        lv_obj_set_style_radius(badge_icon_, 12, 0);
        lv_obj_set_style_bg_color(badge_icon_, kind_col, 0);
        lv_obj_set_style_bg_opa(badge_icon_, 34, 0);

        adw::Widget badge_w(badge_icon_);
        lbl_icon_ = adw::Label(
            badge_w,
            kind_ == ValidatorKind::Bill ? "$" : LV_SYMBOL_LOOP,
            kind_ == ValidatorKind::Bill ? ADW_FONT_TITLE_2 : ADW_FONT_ICONS,
            kind_col);
        lv_obj_center(lbl_icon_.raw());

        adw::Box meta(top_row, adw::Orientation::Vertical, 4);
        meta.set_flex_grow(1);

        adw::Label lbl_role(
            meta,
            kind_ == ValidatorKind::Bill ? "Reciclador de Billetes" : "Reciclador de Monedas",
            ADW_FONT_CAPTION,
            ADW_COLOR_FG);
        lbl_role.set_color(ADW_COLOR_FG, ADW_OPA_DIM);

        v_lbl_titulo_ = adw::Label(meta, "Sin Definir", ADW_FONT_HEADING, ADW_COLOR_FG);
        v_lbl_titulo_.set_hexpand(true);
        lv_label_set_long_mode(v_lbl_titulo_.raw(), LV_LABEL_LONG_DOT);

        // Píldora de estado de conexión
        status_pill_ = lv_obj_create(hero.raw());
        lv_obj_remove_style_all(status_pill_);
        adw_set_scrollable(status_pill_, false);
        lv_obj_set_size(status_pill_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(status_pill_, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(status_pill_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(status_pill_, 10, 0);
        lv_obj_set_style_pad_ver(status_pill_, 4, 0);
        lv_obj_set_style_pad_column(status_pill_, 7, 0);
        lv_obj_set_style_radius(status_pill_, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(status_pill_, ADW_COLOR_ERROR_TEXT, 0);
        lv_obj_set_style_bg_opa(status_pill_, 30, 0);

        status_dot_ = lv_obj_create(status_pill_);
        lv_obj_remove_style_all(status_dot_);
        lv_obj_set_size(status_dot_, 8, 8);
        lv_obj_set_style_radius(status_dot_, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(status_dot_, ADW_COLOR_ERROR_TEXT, 0);
        lv_obj_set_style_bg_opa(status_dot_, LV_OPA_COVER, 0);

        adw::Widget st_w(status_pill_);
        lbl_status_ = adw::Label(st_w, "Desconectado", ADW_FONT_CAPTION, ADW_COLOR_ERROR_TEXT);

        // Franja de error / diagnóstico
        error_box_ = lv_obj_create(hero.raw());
        lv_obj_remove_style_all(error_box_);
        adw_set_scrollable(error_box_, false);
        lv_obj_set_size(error_box_, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(error_box_, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(error_box_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(error_box_, 10, 0);
        lv_obj_set_style_pad_ver(error_box_, 8, 0);
        lv_obj_set_style_pad_column(error_box_, 8, 0);
        lv_obj_set_style_radius(error_box_, 9, 0);
        lv_obj_set_style_bg_color(error_box_, ADW_COLOR_ERROR_TEXT, 0);
        lv_obj_set_style_bg_opa(error_box_, 22, 0);
        lv_obj_set_style_border_width(error_box_, 1, 0);
        lv_obj_set_style_border_color(error_box_, ADW_COLOR_ERROR_TEXT, 0);
        lv_obj_set_style_border_opa(error_box_, 70, 0);

        adw::Widget err_w(error_box_);
        lbl_err_ico_ = adw::Label(err_w, LV_SYMBOL_WARNING, ADW_FONT_ICONS, ADW_COLOR_ERROR_TEXT);
        lbl_err_txt_ = adw::Label(err_w, "Sin conexión", ADW_FONT_CAPTION, ADW_COLOR_ERROR_TEXT);
        lbl_err_txt_.set_flex_grow(1);
        lbl_err_txt_.set_wrap(true);

        // ---- 2. Tarjeta de puerto serial y dirección SSP ----
        auto port_card = create_card(left, 14, 10);

        adw::Box port_hdr(port_card, adw::Orientation::Horizontal, 8);
        port_hdr.set_align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        adw::Label lbl_port_t(port_hdr, "Puerto Serial", ADW_FONT_HEADING, ADW_COLOR_FG);
        lbl_port_t.set_flex_grow(1);

        // Badge SSP 0 / SSP 16
        lv_obj_t* ssp_pill = lv_obj_create(port_hdr.raw());
        lv_obj_remove_style_all(ssp_pill);
        adw_set_scrollable(ssp_pill, false);
        lv_obj_set_size(ssp_pill, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_radius(ssp_pill, 6, 0);
        lv_obj_set_style_bg_color(ssp_pill, ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(ssp_pill, ADW_OPA_CTRL_BG, 0);
        lv_obj_set_style_pad_hor(ssp_pill, 8, 0);
        lv_obj_set_style_pad_ver(ssp_pill, 3, 0);

        char ssp_buf[16];
        std::snprintf(ssp_buf, sizeof(ssp_buf), "SSP %u", static_cast<unsigned>(ssp_address_));
        adw::Widget ssp_w(ssp_pill);
        adw::Label lbl_ssp_badge(ssp_w, ssp_buf, ADW_FONT_MONO, ADW_COLOR_FG);
        lbl_ssp_badge.set_color(ADW_COLOR_FG, 200);

        // Fila dropdown + botón refrescar puertos
        adw::Box port_ctrl(port_card, adw::Orientation::Horizontal, 8);
        port_ctrl.set_align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        dd_puerto_ = lv_dropdown_create(port_ctrl.raw());
        lv_dropdown_set_options(dd_puerto_, "Buscando...");
        lv_dropdown_set_symbol(dd_puerto_, LV_SYMBOL_DOWN);
        lv_obj_set_flex_grow(dd_puerto_, 1);
        lv_obj_set_style_radius(dd_puerto_, 9, 0);
        lv_obj_set_style_bg_color(dd_puerto_, ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(dd_puerto_, ADW_OPA_CTRL_BG, 0);
        lv_obj_set_style_bg_opa(dd_puerto_, ADW_OPA_CTRL_ACT, LV_STATE_PRESSED);
        lv_obj_set_style_border_width(dd_puerto_, 0, 0);
        lv_obj_set_style_pad_hor(dd_puerto_, 10, 0);
        lv_obj_set_style_pad_ver(dd_puerto_, 9, 0);
        lv_obj_set_style_text_color(dd_puerto_, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_font(dd_puerto_, ADW_FONT_MONO, 0);
        lv_obj_set_style_text_font(dd_puerto_, ADW_FONT_ICONS, LV_PART_INDICATOR);
        lv_obj_set_style_text_opa(dd_puerto_, 200, LV_PART_INDICATOR);
        lv_obj_add_event_cb(dd_puerto_, adw_combo_open_cb, LV_EVENT_READY, nullptr);

        btn_refresh_ports_ = adw::IconButton(port_ctrl, LV_SYMBOL_REFRESH, adw::BtnVariant::Default);

        adw::Label lbl_hint(
            port_card,
            "Al cambiar el puerto, pulsa Reconectar para guardar y aplicar.",
            ADW_FONT_CAPTION,
            ADW_COLOR_FG);
        lbl_hint.set_color(ADW_COLOR_FG, ADW_OPA_DIM);
        lbl_hint.set_hexpand(true);
        lbl_hint.set_wrap(true);
    }

    void build_right_column() {
        adw::ScrollView right(split_, 2);
        right.set_flex_grow(1);
        lv_obj_set_height(right.raw(), lv_pct(100));
        lv_obj_set_style_pad_row(right.raw(), 14, 0);

        // ---- Grupo 1: Protocolo y Estado ----
        list_proto_ = adw::BoxedList(right, "Protocolo y Estado");
        row_id_      = make_compact_row(list_proto_, "ID Dispositivo", true);
        row_ssp_     = make_compact_row(list_proto_, "Versión SSP",    true);
        row_fw_      = make_compact_row(list_proto_, "Firmware",       true);
        row_dataset_ = make_compact_row(list_proto_, "Set Regional",   true);
        row_rtc_     = make_compact_row(list_proto_, "Reloj Interno",  false);

        // ---- Grupo 2: Hardware y Series ----
        list_hw_ = adw::BoxedList(right, "Hardware y Números de Serie");
        row_serial_     = make_compact_row(list_hw_, "ID Serial",        true);
        row_rev_        = make_compact_row(list_hw_, "Revisión",         true);
        row_mod_serial_ = make_compact_row(list_hw_, "ID Módulo",        true);
        row_mod_rev_    = make_compact_row(list_hw_, "Revisión Módulo",  false);
    }

    ValidatorKind kind_;
    uint8_t ssp_address_ = 0;
    uint16_t id_conf_    = 0;
    std::vector<std::string> puertos_;

    adw::Box split_;

    // Columna izquierda
    lv_obj_t* badge_icon_  = nullptr;
    adw::Label lbl_icon_;
    adw::Label v_lbl_titulo_;
    lv_obj_t* status_pill_ = nullptr;
    lv_obj_t* status_dot_  = nullptr;
    adw::Label lbl_status_;
    lv_obj_t* error_box_   = nullptr;
    adw::Label lbl_err_ico_;
    adw::Label lbl_err_txt_;
    lv_obj_t* dd_puerto_   = nullptr;
    adw::IconButton btn_refresh_ports_;

    // Columna derecha
    adw::BoxedList list_proto_;
    adw::ValueRow row_id_;
    adw::ValueRow row_ssp_;
    adw::ValueRow row_fw_;
    adw::ValueRow row_dataset_;
    adw::ValueRow row_rtc_;

    adw::BoxedList list_hw_;
    adw::ValueRow row_serial_;
    adw::ValueRow row_rev_;
    adw::ValueRow row_mod_serial_;
    adw::ValueRow row_mod_rev_;
};

// ============================================================================
// CONTROLADOR / PANTALLA DETALLES VALIDADORES (reemplaza controller/config/validador.cpp)
// ============================================================================
class DetallesValidadorPreview {
public:
    DetallesValidadorPreview()
        : scr_(lv_screen_active())
        , header_(scr_, "Validadores de Efectivo", true)
    {
        btn_reconnect_ = adw::Button(header_, "Reconectar", adw::BtnVariant::Suggested);
        header_.pack_end(btn_reconnect_);

        build_switcher_bar();

        stack_ = adw::Stack(scr_);
        auto page_bill = stack_.add_page("bill", 0);
        auto page_coin = stack_.add_page("coin", 0);

        v_box_bill_ = std::make_unique<VValidador>(page_bill, ValidatorKind::Bill, 0);
        v_box_coin_ = std::make_unique<VValidador>(page_coin, ValidatorKind::Coin, 16);
        v_box_bill_->set_id_conf(1);
        v_box_coin_->set_id_conf(3);

        // Puertos iniciales (simulados en preview, reales desde /dev en Linux)
        auto ports = VValidador::scan_serial_ports();
        v_box_bill_->set_ports(ports, "/dev/ttyUSB0");
        v_box_coin_->set_ports(ports, "/dev/ttyUSB1");

        apply_demo_connection();
        set_active_tab(0);
        connect_signals();
    }

private:
    struct TabBtn {
        lv_obj_t* btn = nullptr;
        lv_obj_t* dot = nullptr;
        adw::Label lbl_name;
        adw::Label lbl_sub;
    };

    TabBtn create_tab(adw::Widget& parent, const char* title, const char* subtitle) {
        TabBtn t;
        t.btn = lv_button_create(parent.raw());
        lv_obj_remove_style_all(t.btn);
        lv_obj_set_flex_grow(t.btn, 1);
        lv_obj_set_height(t.btn, 40);
        lv_obj_set_flex_flow(t.btn, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(t.btn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(t.btn, 12, 0);
        lv_obj_set_style_pad_column(t.btn, 8, 0);
        lv_obj_set_style_radius(t.btn, 9, 0);
        lv_obj_set_style_bg_color(t.btn, ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(t.btn, LV_OPA_TRANSP, 0);

        t.dot = lv_obj_create(t.btn);
        lv_obj_remove_style_all(t.dot);
        adw_set_clickable(t.dot, false);
        lv_obj_set_size(t.dot, 8, 8);
        lv_obj_set_style_radius(t.dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(t.dot, ADW_COLOR_SUCCESS_TEXT, 0);
        lv_obj_set_style_bg_opa(t.dot, LV_OPA_COVER, 0);

        adw::Widget btn_w(t.btn);
        t.lbl_name = adw::Label(btn_w, title, ADW_FONT_HEADING, ADW_COLOR_FG);
        adw_set_clickable(t.lbl_name.raw(), false);

        t.lbl_sub = adw::Label(btn_w, subtitle, ADW_FONT_CAPTION, ADW_COLOR_FG);
        t.lbl_sub.set_color(ADW_COLOR_FG, ADW_OPA_DIM);
        adw_set_clickable(t.lbl_sub.raw(), false);
        return t;
    }

    void build_switcher_bar() {
        adw::Box bar_wrap(scr_, adw::Orientation::Horizontal, 0);
        bar_wrap.set_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_top(bar_wrap.raw(), 12, 0);
        lv_obj_set_style_pad_hor(bar_wrap.raw(), 14, 0);

        adw::Box pill(bar_wrap, adw::Orientation::Horizontal, 4);
        pill.set_width(lv_pct(100));
        pill.set_padding(4);
        lv_obj_set_style_bg_color(pill.raw(), ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(pill.raw(), 16, 0);
        lv_obj_set_style_radius(pill.raw(), 12, 0);
        lv_obj_set_style_border_width(pill.raw(), 1, 0);
        lv_obj_set_style_border_color(pill.raw(), ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(pill.raw(), 40, 0);

        tab_bill_ = create_tab(pill, "Billetes", "NV200");
        tab_coin_ = create_tab(pill, "Monedas",  "SMART Hopper");
    }

    void set_active_tab(int idx) {
        auto style_tab = [](TabBtn& t, bool active) {
            lv_obj_set_style_bg_opa(t.btn, active ? 34 : LV_OPA_TRANSP, 0);
            t.lbl_name.set_color(ADW_COLOR_FG, active ? LV_OPA_COVER : ADW_OPA_DIM);
        };
        style_tab(tab_bill_, idx == 0);
        style_tab(tab_coin_, idx == 1);
        stack_.set_visible_child(idx == 0 ? "bill" : "coin");
    }

    void update_tab_status(TabBtn& t, const ValidatorInfo& info) {
        bool ok = info.is_open && info.device_model != "UNKNOWN";
        lv_obj_set_style_bg_color(t.dot, ok ? ADW_COLOR_SUCCESS_TEXT : ADW_COLOR_ERROR_TEXT, 0);
        t.lbl_sub.set_text(ok ? info.device_model : "Sin conexión");
    }

    void connect_signals() {
        header_.signal_back_clicked().connect([]() {
            adw::Toast::show("Regresando a Configuración...", 1600);
        });

        adw::detail::connect_raw(tab_bill_.btn, LV_EVENT_CLICKED, [this](lv_event_t*) {
            set_active_tab(0);
        });
        adw::detail::connect_raw(tab_coin_.btn, LV_EVENT_CLICKED, [this](lv_event_t*) {
            set_active_tab(1);
        });

        auto on_refresh = [this]() {
            auto ports = VValidador::scan_serial_ports();
            v_box_bill_->set_ports(ports, v_box_bill_->get_puerto_seleccionado());
            v_box_coin_->set_ports(ports, v_box_coin_->get_puerto_seleccionado());
            adw::Toast::show("Puertos seriales actualizados (" + std::to_string(ports.size()) + ")", 1600);
        };
        v_box_bill_->signal_refresh_ports().connect(on_refresh);
        v_box_coin_->signal_refresh_ports().connect(on_refresh);

        btn_reconnect_.signal_clicked().connect([this]() {
            std::string p_bill = v_box_bill_->get_puerto_seleccionado();
            std::string p_coin = v_box_coin_->get_puerto_seleccionado();

            if (!p_bill.empty() && p_bill == p_coin) {
                adw::Toast::show("Billetes y Monedas no pueden usar el mismo puerto", 2400);
                return;
            }

            // En producción: guarda en SQLite y lanza conecta_validadores() en std::thread
            // sin sleep_for() en el hilo gráfico; al terminar llama adw::post_to_ui(...)
            loading_.show("Reiniciando y reconectando validadores...");
            reconnect_timer_.start(1100, [this]() {
                loading_.close();
                apply_demo_connection();
                return false;
            });
        });
    }

    void apply_demo_connection() {
        ValidatorInfo bill;
        if (v_box_bill_->get_puerto_seleccionado() == "/dev/ttyUSB3") {
            bill.device_model    = "UNKNOWN";
            bill.is_open         = false;
            bill.device_error    = "Timeout SSP en /dev/ttyUSB3";
        } else {
            bill.device_model    = "NV200 Spectral";
            bill.device_id       = "0x07A4";
            bill.is_open         = true;
            bill.ssp_version     = 8;
            bill.device_error    = "NONE";
            bill.firmware        = "4.42.0";
            bill.dataset         = "MXN-024";
            bill.serial          = "84219034";
            bill.revision        = "Rev. C";
            bill.module_serial   = "PM-771204";
            bill.module_revision = "Rev. 2.1";
            bill.real_time_clock = "2026-11-24 16:25";
        }

        ValidatorInfo coin;
        if (v_box_coin_->get_puerto_seleccionado() == "/dev/ttyUSB3") {
            coin.device_model    = "UNKNOWN";
            coin.is_open         = false;
            coin.device_error    = "Hopper no responde en /dev/ttyUSB3";
        } else {
            coin.device_model    = "SMART Hopper";
            coin.device_id       = "0x10B2";
            coin.is_open         = true;
            coin.ssp_version     = 8;
            coin.device_error    = "NONE";
            coin.firmware        = "3.18.2";
            coin.dataset         = "MXN-COIN-09";
            coin.serial          = "55019284";
            coin.revision        = "Rev. B";
            coin.module_serial   = "CF-992011";
            coin.module_revision = "Rev. 1.4";
            coin.real_time_clock = "2026-11-24 16:25";
        }

        v_box_bill_->set_data(bill);
        v_box_coin_->set_data(coin);
        update_tab_status(tab_bill_, bill);
        update_tab_status(tab_coin_, coin);

        if (bill.is_open && coin.is_open) {
            adw::Toast::show("Validadores conectados correctamente", 2000);
        } else {
            adw::Toast::show("Revisa el puerto del validador desconectado", 2400);
        }
    }

    adw::Screen scr_;
    adw::HeaderBar header_;
    adw::Button btn_reconnect_;

    TabBtn tab_bill_;
    TabBtn tab_coin_;
    adw::Stack stack_;

    std::unique_ptr<VValidador> v_box_bill_;
    std::unique_ptr<VValidador> v_box_coin_;

    adw::LoadingOverlay loading_;
    adw::Timer reconnect_timer_;
};

static std::unique_ptr<DetallesValidadorPreview> g_validador_app;

static void ui_init(void) {
    adw::UiDispatcher::init();
    g_validador_app.reset();
    g_validador_app = std::make_unique<DetallesValidadorPreview>();
}

#ifdef LVGL_LIVE_PREVIEW
extern "C" void lvgl_live_preview_init(void) {
    ui_init();
}
#endif
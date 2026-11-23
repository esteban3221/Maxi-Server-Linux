#include "lvgl.h"
#include "adw_lvgl.h"

typedef struct {
    lv_obj_t *stack_wifi;
    lv_obj_t *btn_redes;
    lv_obj_t *btn_red;
    lv_obj_t *lbl_ssid_wifi;
    lv_obj_t *lbl_ip_wifi;
    lv_obj_t *lbl_mac_wifi;
    lv_obj_t *lbl_ip_eth;
    lv_obj_t *lbl_mac_eth;
    lv_obj_t *btn_regresar;
    lv_obj_t *btn_refresh;
    lv_obj_t *list_box_wifi;
} ui_red_page_t;

/* Instancia persistente (nunca en stack) para los callbacks */
static ui_red_page_t g_red_ui;

/* Helpers locales compatibles con tu adw_lvgl.h actual */
static lv_obj_t *make_warning_button(lv_obj_t *parent, const char *texto) {
    lv_obj_t *btn = adw_button_create(parent, texto, ADW_BTN_DEFAULT, false);
    lv_obj_set_style_bg_color(btn, ADW_COLOR_WARNING_TEXT, 0);
    lv_obj_set_style_bg_opa(btn, ADW_OPA_DESTR_BG, 0);
    lv_obj_set_style_bg_opa(btn, ADW_OPA_DESTR_ACT, LV_STATE_PRESSED);
    lv_obj_t *lbl = lv_obj_get_child(btn, 0);
    if (lbl) lv_obj_set_style_text_color(lbl, ADW_COLOR_WARNING_TEXT, 0);
    return btn;
}

static lv_obj_t *make_icon_button(lv_obj_t *parent, const char *symbol) {
    lv_obj_t *btn = adw_button_create(parent, symbol, ADW_BTN_DEFAULT, false);
    lv_obj_set_size(btn, 38, 38);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_t *lbl = lv_obj_get_child(btn, 0);
    if (lbl) lv_obj_set_style_text_font(lbl, ADW_FONT_ICONS, 0);
    return btn;
}

static lv_obj_t *make_section_header(lv_obj_t *parent, const char *titulo,
                                     const char *btn_txt, bool is_warning,
                                     int32_t margin_top, lv_obj_t **out_btn) {
    lv_obj_t *hdr = lv_obj_create(parent);
    lv_obj_remove_style_all(hdr);
    adw_set_scrollable(hdr, false);
    lv_obj_set_size(hdr, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(hdr, 10, 0);
    lv_obj_set_style_margin_top(hdr, margin_top, 0);
    lv_obj_set_style_margin_bottom(hdr, 6, 0);

    lv_obj_t *lbl = lv_label_create(hdr);
    lv_label_set_text(lbl, titulo);
    lv_obj_set_flex_grow(lbl, 1);
    lv_obj_set_style_text_font(lbl, ADW_FONT_TITLE_2, 0);
    lv_obj_set_style_text_color(lbl, ADW_COLOR_FG, 0);

    *out_btn = is_warning
        ? make_warning_button(hdr, btn_txt)
        : adw_button_create(hdr, btn_txt, ADW_BTN_SUGGESTED, false);
    return hdr;
}

static lv_obj_t *make_dim_value_row(lv_obj_t *list, const char *titulo,
                                    const char *valor, bool divisor) {
    lv_obj_t *lbl = adw_value_row_create(list, titulo, NULL, valor,
                                         ADW_COLOR_FG, ADW_FONT_BODY, divisor);
    lv_obj_set_style_text_opa(lbl, ADW_OPA_DIM, 0);
    return lbl;
}

/* ==================== CALLBACKS DE PRUEBA INTERACTIVA ==================== */

static void wifi_row_clicked_cb(lv_event_t *e) {
    const char *ssid = (const char *)lv_event_get_user_data(e);
    if (!ssid) return;

    lv_label_set_text(g_red_ui.lbl_ssid_wifi, ssid);
    lv_label_set_text(g_red_ui.lbl_ip_wifi, "192.168.1.48");
    lv_tileview_set_tile_by_index(g_red_ui.stack_wifi, 0, 0, LV_ANIM_ON);
    adw_toast_show("Conectado a la red Wi-Fi", 2200);
}

static void populate_mock_wifi_list(ui_red_page_t *ui) {
    static const char *const ssids[] = {
        "Maxicajero_Oficina_5G",
        "TP-Link_Caja_02",
        "Totalplay-9F4A",
        "INFINITUM_2.4G",
        "Invitados_Tienda",
        "Red_Respaldo_LTE"
    };
    static const char *const rssis[] = {
        "-48 dBm", "-56 dBm", "-67 dBm", "-72 dBm", "-79 dBm", "-84 dBm"
    };

    lv_obj_clean(ui->list_box_wifi);
    size_t count = sizeof(ssids) / sizeof(ssids[0]);
    for (size_t i = 0; i < count; i++) {
        bool divisor = (i + 1 < count);
        lv_obj_t *row = adw_nav_row_create(ui->list_box_wifi, LV_SYMBOL_WIFI,
                                           ssids[i], "WPA2-PSK", rssis[i], divisor);
        lv_obj_add_event_cb(row, wifi_row_clicked_cb, LV_EVENT_CLICKED, (void *)ssids[i]);
    }
}

static void nav_to_redes_cb(lv_event_t *e) {
    ui_red_page_t *ui = (ui_red_page_t *)lv_event_get_user_data(e);
    populate_mock_wifi_list(ui);
    lv_tileview_set_tile_by_index(ui->stack_wifi, 1, 0, LV_ANIM_ON);
}

static void nav_to_status_cb(lv_event_t *e) {
    ui_red_page_t *ui = (ui_red_page_t *)lv_event_get_user_data(e);
    lv_tileview_set_tile_by_index(ui->stack_wifi, 0, 0, LV_ANIM_ON);
}

static void refresh_wifi_cb(lv_event_t *e) {
    ui_red_page_t *ui = (ui_red_page_t *)lv_event_get_user_data(e);
    populate_mock_wifi_list(ui);
    adw_toast_show("Escaneando redes disponibles...", 1800);
}

static void eth_confirm_cb(void *user_data) {
    LV_UNUSED(user_data);
    lv_label_set_text(g_red_ui.lbl_ip_eth, "192.168.10.25");
    adw_toast_show("DHCP renovado en Ethernet", 2000);
}

static void btn_eth_cb(lv_event_t *e) {
    LV_UNUSED(e);
    adw_alert_dialog_show("Gestionar Ethernet",
                          "¿Deseas renovar la dirección IP por DHCP en la interfaz cableada?",
                          "Cancelar", "Renovar", ADW_BTN_SUGGESTED,
                          eth_confirm_cb, NULL);
}

/* ==================== CONSTRUCCIÓN DE PAGE_5 ==================== */

void ui_red_page_create(lv_obj_t *parent, ui_red_page_t *ui) {
    /* GtkStack con transición slide izquierda/derecha */
    ui->stack_wifi = lv_tileview_create(parent);
    lv_obj_set_width(ui->stack_wifi, lv_pct(100));
    lv_obj_set_flex_grow(ui->stack_wifi, 1);
    lv_obj_set_style_bg_opa(ui->stack_wifi, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollbar_mode(ui->stack_wifi, LV_SCROLLBAR_MODE_OFF);

    /* ---------- Página 1: status_red ---------- */
    lv_obj_t *t_status = lv_tileview_add_tile(ui->stack_wifi, 0, 0, LV_DIR_NONE);
    lv_obj_set_scroll_dir(t_status, LV_DIR_VER);
    lv_obj_set_flex_flow(t_status, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(t_status, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(t_status, 24, 0);
    lv_obj_set_style_pad_ver(t_status, 16, 0);
    lv_obj_set_style_pad_row(t_status, 8, 0);

    lv_obj_t *clamp_status = lv_obj_create(t_status);
    lv_obj_remove_style_all(clamp_status);
    adw_set_scrollable(clamp_status, false);
    lv_obj_set_size(clamp_status, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_max_width(clamp_status, 640, 0);
    lv_obj_set_flex_flow(clamp_status, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(clamp_status, 8, 0);

    make_section_header(clamp_status, "Inalámbrica", "Conectar Wi-Fi",
                        false, 0, &ui->btn_redes);
    lv_obj_t *list_wifi = adw_boxed_list_create(clamp_status);
    ui->lbl_ssid_wifi = make_dim_value_row(list_wifi, "Red", "Maxicajero_Oficina_5G", true);
    ui->lbl_ip_wifi   = make_dim_value_row(list_wifi, "Dirección IP", "192.168.1.45", true);
    ui->lbl_mac_wifi  = make_dim_value_row(list_wifi, "Dirección MAC", "A4:CF:12:88:1B:09", false);

    make_section_header(clamp_status, "Ethernet", "Gestionar conexiones",
                        true, 16, &ui->btn_red);
    lv_obj_t *list_eth = adw_boxed_list_create(clamp_status);
    ui->lbl_ip_eth  = make_dim_value_row(list_eth, "Dirección IP", "192.168.10.20", true);
    ui->lbl_mac_eth = make_dim_value_row(list_eth, "Dirección MAC", "DC:A6:32:44:91:F2", false);

    /* ---------- Página 2: redes_wifi ---------- */
    lv_obj_t *t_redes = lv_tileview_add_tile(ui->stack_wifi, 1, 0, LV_DIR_NONE);
    adw_set_scrollable(t_redes, false);
    lv_obj_set_flex_flow(t_redes, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(t_redes, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(t_redes, 24, 0);
    lv_obj_set_style_pad_ver(t_redes, 16, 0);
    lv_obj_set_style_pad_row(t_redes, 12, 0);

    lv_obj_t *top = lv_obj_create(t_redes);
    lv_obj_remove_style_all(top);
    adw_set_scrollable(top, false);
    lv_obj_set_size(top, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_max_width(top, 640, 0);
    lv_obj_set_flex_flow(top, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(top, 12, 0);

    ui->btn_regresar = make_icon_button(top, LV_SYMBOL_LEFT);

    lv_obj_t *lbl_disp = lv_label_create(top);
    lv_label_set_text(lbl_disp, "Redes disponibles");
    lv_obj_set_flex_grow(lbl_disp, 1);
    lv_obj_set_style_text_font(lbl_disp, ADW_FONT_TITLE_2, 0);
    lv_obj_set_style_text_color(lbl_disp, ADW_COLOR_FG, 0);

    ui->btn_refresh = make_icon_button(top, LV_SYMBOL_REFRESH);

    /* GtkScrolledWindow + list_box_wifi */
    lv_obj_t *scroll = lv_obj_create(t_redes);
    lv_obj_remove_style_all(scroll);
    lv_obj_set_scroll_dir(scroll, LV_DIR_VER);
    lv_obj_set_width(scroll, lv_pct(100));
    lv_obj_set_style_max_width(scroll, 640, 0);
    lv_obj_set_flex_grow(scroll, 1);
    lv_obj_set_flex_flow(scroll, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_ver(scroll, 4, 0);

    ui->list_box_wifi = adw_boxed_list_create(scroll);
    populate_mock_wifi_list(ui);

    /* Eventos */
    lv_obj_add_event_cb(ui->btn_redes, nav_to_redes_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->btn_regresar, nav_to_status_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->btn_refresh, refresh_wifi_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->btn_red, btn_eth_cb, LV_EVENT_CLICKED, ui);
}

static void ui_init(void) {
    lv_obj_t *scr = adw_screen_prepare(lv_screen_active());
    lv_obj_t *btn_back = NULL;
    adw_headerbar_create(scr, "Configuración de red", &btn_back);

    ui_red_page_create(scr, &g_red_ui);
    adw_keyboard_init(scr);
}

#ifdef LVGL_LIVE_PREVIEW
void lvgl_live_preview_init(void) {
    ui_init();
}
#endif

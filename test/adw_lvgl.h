#pragma once

#include "lvgl.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ADW_SEL(part, state) ((lv_style_selector_t)(part) | (lv_style_selector_t)(state))

/* ==================== FUENTES ESPAÑOL (LATIN-1) ==================== */
/* ==================== FUENTES ESPAÑOL (LATIN-1) ==================== */
#if defined(__has_include) && __has_include("font_adw_14.h")
#include "font_adw_14.h"
#include "font_adw_16.h"
#include "font_adw_16_bold.h"
#include "font_adw_22_bold.h"
#include "font_adw_28_bold.h"
#include "font_adw_mono_16.h"
#include "font_adw_mono_22.h"
#define ADW_HAS_CUSTOM_FONTS 1
#elif defined(__has_include) && __has_include("../assets/fonts/font_adw_14.h")
#include "../assets/fonts/font_adw_14.h"
#include "../assets/fonts/font_adw_16.h"
#include "../assets/fonts/font_adw_16_bold.h"
#include "../assets/fonts/font_adw_22_bold.h"
#include "../assets/fonts/font_adw_28_bold.h"
#include "../assets/fonts/font_adw_mono_16.h"
#include "../assets/fonts/font_adw_mono_22.h"
#define ADW_HAS_CUSTOM_FONTS 1
#else
#define ADW_HAS_CUSTOM_FONTS 0
#endif

#if ADW_HAS_CUSTOM_FONTS
#define ADW_FONT_TITLE_1 (&font_adw_28_bold)
#define ADW_FONT_TITLE_2 (&font_adw_22_bold)
#define ADW_FONT_TITLE_3 (&font_adw_22_bold)
#define ADW_FONT_HEADING (&font_adw_16_bold)
#define ADW_FONT_BODY (&font_adw_16)
#define ADW_FONT_CAPTION (&font_adw_14)
#define ADW_FONT_MONO (&font_adw_mono_16)
#define ADW_FONT_MONO_LG (&font_adw_mono_22)
#else
#define ADW_FONT_TITLE_1 (LV_FONT_DEFAULT)
#define ADW_FONT_TITLE_2 (LV_FONT_DEFAULT)
#define ADW_FONT_TITLE_3 (LV_FONT_DEFAULT)
#define ADW_FONT_HEADING (LV_FONT_DEFAULT)
#define ADW_FONT_BODY (LV_FONT_DEFAULT)
#define ADW_FONT_CAPTION (LV_FONT_DEFAULT)
#define ADW_FONT_MONO (LV_FONT_DEFAULT)
#define ADW_FONT_MONO_LG (LV_FONT_DEFAULT)
#endif
#define ADW_FONT_ICONS (LV_FONT_DEFAULT)

/* ==================== PALETA ADWAITA DARK ==================== */
#define ADW_COLOR_WINDOW_BG lv_color_hex(0x222226)
#define ADW_COLOR_VIEW_BG lv_color_hex(0x1D1D20)
#define ADW_COLOR_HEADERBAR_BG lv_color_hex(0x2E2E32)
#define ADW_COLOR_DIALOG_BG lv_color_hex(0x36363A)
#define ADW_COLOR_TOAST_BG lv_color_hex(0x505053)
#define ADW_COLOR_FG lv_color_hex(0xFFFFFF)
#define ADW_COLOR_SHADE lv_color_hex(0x000006)

#define ADW_COLOR_ACCENT_BG lv_color_hex(0x3584E4)
#define ADW_COLOR_ACCENT_ACT lv_color_hex(0x2A69B6)
#define ADW_COLOR_ACCENT_TEXT lv_color_hex(0x78AEED)
#define ADW_COLOR_SUCCESS_TEXT lv_color_hex(0x8FF0A4)
#define ADW_COLOR_WARNING_TEXT lv_color_hex(0xF8E45C)
#define ADW_COLOR_ERROR_TEXT lv_color_hex(0xFF7B63)

#define ADW_OPA_CARD_BG 20   /* 8%  */
#define ADW_OPA_CARD_ACT 40  /* 16% */
#define ADW_OPA_CTRL_BG 26   /* 10% */
#define ADW_OPA_CTRL_ACT 76  /* 30% */
#define ADW_OPA_BORDER 38    /* 15% */
#define ADW_OPA_SHADE 92     /* 36% */
#define ADW_OPA_DIM 140      /* 55% */
#define ADW_OPA_DESTR_BG 38  /* 15% */
#define ADW_OPA_DESTR_ACT 89 /* 35% */

#ifdef __cplusplus
#define ADW_GLOBAL inline
#else
#define ADW_GLOBAL static
#endif

    typedef enum
    {
        ADW_BTN_DEFAULT = 0,
        ADW_BTN_SUGGESTED,
        ADW_BTN_DESTRUCTIVE,
        ADW_BTN_FLAT,
        ADW_BTN_WARNING
    } adw_btn_variant_t;

    typedef void (*adw_dialog_cb_t)(void *user_data);
    static inline bool adw_is_hidden(const lv_obj_t *obj)
    {
#if (LVGL_VERSION_MAJOR > 9) || (LVGL_VERSION_MAJOR == 9 && LVGL_VERSION_MINOR >= 6)
        return lv_obj_is_hidden(obj);
#else
    return lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN);
#endif
    }

    static inline void adw_set_scroll_one(lv_obj_t *obj, bool en)
    {
#if (LVGL_VERSION_MAJOR > 9) || (LVGL_VERSION_MAJOR == 9 && LVGL_VERSION_MINOR >= 6)
        lv_obj_set_scroll_one(obj, en);
#else
    if (en)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ONE);
    else
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_ONE);
#endif
    }

    /* ==================== COMPATIBILIDAD LVGL 9.2 - 9.6+ ==================== */
    static inline void adw_set_hidden(lv_obj_t *obj, bool en)
    {
#if (LVGL_VERSION_MAJOR > 9) || (LVGL_VERSION_MAJOR == 9 && LVGL_VERSION_MINOR >= 6)
        lv_obj_set_hidden(obj, en);
#else
    if (en)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN);
#endif
    }

    static inline void adw_set_scrollable(lv_obj_t *obj, bool en)
    {
#if (LVGL_VERSION_MAJOR > 9) || (LVGL_VERSION_MAJOR == 9 && LVGL_VERSION_MINOR >= 6)
        lv_obj_set_scrollable(obj, en);
#else
    if (en)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    else
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
#endif
    }

    static inline void adw_set_clickable(lv_obj_t *obj, bool en)
    {
#if (LVGL_VERSION_MAJOR > 9) || (LVGL_VERSION_MAJOR == 9 && LVGL_VERSION_MINOR >= 6)
        lv_obj_set_clickable(obj, en);
#else
    if (en)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    else
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
#endif
    }

    /* ==================== TECLADO GLOBAL AUTOMATICO ==================== */
    ADW_GLOBAL lv_obj_t *adw_global_kb = NULL;

    static inline void adw_kb_focus_cb(lv_event_t *e)
    {
        if (!adw_global_kb)
            return;
        lv_event_code_t code = lv_event_get_code(e);
        lv_obj_t *ta = lv_event_get_target_obj(e);
        lv_keyboard_mode_t mode = (lv_keyboard_mode_t)(uintptr_t)lv_event_get_user_data(e);

        if (code == LV_EVENT_FOCUSED)
        {
            lv_keyboard_set_mode(adw_global_kb, mode);
            lv_keyboard_set_textarea(adw_global_kb, ta);
            adw_set_hidden(adw_global_kb, false);
            lv_obj_move_foreground(adw_global_kb);
            lv_obj_scroll_to_view_recursive(ta, LV_ANIM_ON);
        }
        else if (code == LV_EVENT_DEFOCUSED || code == LV_EVENT_READY || code == LV_EVENT_CANCEL)
        {
            lv_keyboard_set_textarea(adw_global_kb, NULL);
            adw_set_hidden(adw_global_kb, true);
        }
    }

    static inline lv_obj_t *adw_keyboard_init(lv_obj_t *parent)
    {
        adw_global_kb = lv_keyboard_create(parent);
        lv_obj_set_size(adw_global_kb, lv_pct(100), lv_pct(45));
        lv_obj_align(adw_global_kb, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_bg_color(adw_global_kb, ADW_COLOR_HEADERBAR_BG, 0);
        lv_obj_set_style_border_width(adw_global_kb, 1, 0);
        lv_obj_set_style_border_side(adw_global_kb, LV_BORDER_SIDE_TOP, 0);
        lv_obj_set_style_border_color(adw_global_kb, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(adw_global_kb, ADW_OPA_SHADE, 0);
        adw_set_hidden(adw_global_kb, true);
        return adw_global_kb;
    }

    static inline void adw_bind_keyboard(lv_obj_t *textarea, lv_keyboard_mode_t mode)
    {
        lv_obj_add_event_cb(textarea, adw_kb_focus_cb, LV_EVENT_ALL, (void *)(uintptr_t)mode);
    }

    /* ==================== PANTALLA, HEADERBAR Y PAGE ==================== */
    static inline lv_obj_t *adw_screen_prepare(lv_obj_t *scr)
    {
        lv_obj_clean(scr);
        adw_global_kb = NULL;
        lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_style_bg_color(scr, ADW_COLOR_WINDOW_BG, 0);
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
        lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_all(scr, 0, 0);
        lv_obj_set_style_pad_row(scr, 0, 0);
        return scr;
    }

    static inline lv_obj_t *adw_headerbar_create(lv_obj_t *parent, const char *titulo,
                                                 lv_obj_t **out_btn_back)
    {
        lv_obj_t *bar = lv_obj_create(parent);
        lv_obj_remove_style_all(bar);
        adw_set_scrollable(bar, false);
        lv_obj_set_size(bar, lv_pct(100), 52);
        lv_obj_set_style_bg_color(bar, ADW_COLOR_HEADERBAR_BG, 0);
        lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(bar, 1, 0);
        lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(bar, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(bar, ADW_OPA_SHADE, 0);
        lv_obj_set_style_pad_hor(bar, 12, 0);

        if (out_btn_back)
        {
            lv_obj_t *btn = lv_button_create(bar);
            lv_obj_remove_style_all(btn);
            lv_obj_set_size(btn, 36, 36);
            lv_obj_align(btn, LV_ALIGN_LEFT_MID, 0, 0);
            lv_obj_set_style_radius(btn, 9, 0);
            lv_obj_set_style_bg_color(btn, ADW_COLOR_FG, LV_STATE_PRESSED);
            lv_obj_set_style_bg_opa(btn, 40, LV_STATE_PRESSED);

            lv_obj_t *ico = lv_label_create(btn);
            lv_label_set_text(ico, LV_SYMBOL_LEFT);
            lv_obj_set_style_text_font(ico, ADW_FONT_ICONS, 0);
            lv_obj_set_style_text_color(ico, ADW_COLOR_FG, 0);
            lv_obj_center(ico);
            *out_btn_back = btn;
        }

        lv_obj_t *lbl = lv_label_create(bar);
        lv_label_set_text(lbl, titulo);
        lv_obj_set_style_text_color(lbl, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_font(lbl, ADW_FONT_HEADING, 0);
        lv_obj_center(lbl);
        return bar;
    }

    static inline lv_obj_t *adw_page_create(lv_obj_t *parent, int32_t max_width)
    {
        lv_obj_t *scroll = lv_obj_create(parent);
        lv_obj_remove_style_all(scroll);
        lv_obj_set_width(scroll, lv_pct(100));
        lv_obj_set_flex_grow(scroll, 1);
        lv_obj_set_flex_flow(scroll, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(scroll, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(scroll, 28, 0);
        lv_obj_set_style_pad_ver(scroll, 20, 0);
        lv_obj_set_style_pad_row(scroll, 18, 0);

        if (max_width > 0)
        {
            lv_obj_t *clamp = lv_obj_create(scroll);
            lv_obj_remove_style_all(clamp);
            adw_set_scrollable(clamp, false);
            lv_obj_set_size(clamp, lv_pct(100), LV_SIZE_CONTENT);
            lv_obj_set_style_max_width(clamp, max_width, 0);
            lv_obj_set_flex_flow(clamp, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_style_pad_row(clamp, 18, 0);
            return clamp;
        }
        return scroll;
    }

    /* ==================== BOTONES ==================== */
    static inline lv_obj_t *adw_button_create(lv_obj_t *parent, const char *texto,
                                              adw_btn_variant_t variant, bool pill)
    {
        lv_obj_t *btn = lv_button_create(parent);
        lv_obj_remove_style_all(btn);
        lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_radius(btn, pill ? LV_RADIUS_CIRCLE : 9, 0);
        lv_obj_set_style_pad_hor(btn, pill ? 32 : 18, 0);
        lv_obj_set_style_pad_ver(btn, pill ? 10 : 8, 0);
        lv_obj_set_style_min_height(btn, 36, 0);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, texto);
        lv_obj_set_style_text_font(lbl, ADW_FONT_HEADING, 0);
        lv_obj_center(lbl);

        switch (variant)
        {
        case ADW_BTN_SUGGESTED:
            lv_obj_set_style_bg_color(btn, ADW_COLOR_ACCENT_BG, 0);
            lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
            lv_obj_set_style_bg_color(btn, ADW_COLOR_ACCENT_ACT, LV_STATE_PRESSED);
            lv_obj_set_style_text_color(lbl, ADW_COLOR_FG, 0);
            break;
        case ADW_BTN_DESTRUCTIVE:
            lv_obj_set_style_bg_color(btn, ADW_COLOR_ERROR_TEXT, 0);
            lv_obj_set_style_bg_opa(btn, ADW_OPA_DESTR_BG, 0);
            lv_obj_set_style_bg_opa(btn, ADW_OPA_DESTR_ACT, LV_STATE_PRESSED);
            lv_obj_set_style_text_color(lbl, ADW_COLOR_ERROR_TEXT, 0);
            break;
        case ADW_BTN_FLAT:
            lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
            lv_obj_set_style_bg_color(btn, ADW_COLOR_FG, LV_STATE_PRESSED);
            lv_obj_set_style_bg_opa(btn, 40, LV_STATE_PRESSED);
            lv_obj_set_style_text_color(lbl, ADW_COLOR_FG, 0);
            break;
        case ADW_BTN_WARNING:
            lv_obj_set_style_bg_color(btn, ADW_COLOR_WARNING_TEXT, 0);
            lv_obj_set_style_bg_opa(btn, ADW_OPA_DESTR_BG, 0);
            lv_obj_set_style_bg_opa(btn, ADW_OPA_DESTR_ACT, LV_STATE_PRESSED);
            lv_obj_set_style_text_color(lbl, ADW_COLOR_WARNING_TEXT, 0);
            break;
        case ADW_BTN_DEFAULT:
        default:
            lv_obj_set_style_bg_color(btn, ADW_COLOR_FG, 0);
            lv_obj_set_style_bg_opa(btn, ADW_OPA_CTRL_BG, 0);
            lv_obj_set_style_bg_opa(btn, ADW_OPA_CTRL_ACT, LV_STATE_PRESSED);
            lv_obj_set_style_text_color(lbl, ADW_COLOR_FG, 0);
            break;
        }
        return btn;
    }

    static inline lv_obj_t *adw_icon_button_create(lv_obj_t *parent, const char *symbol,
                                                   adw_btn_variant_t variant)
    {
        lv_obj_t *btn = adw_button_create(parent, symbol, variant, false);
        lv_obj_set_size(btn, 38, 38);
        lv_obj_set_style_pad_all(btn, 0, 0);
        lv_obj_t *lbl = lv_obj_get_child(btn, 0);
        if (lbl)
            lv_obj_set_style_text_font(lbl, ADW_FONT_ICONS, 0);
        return btn;
    }

    /* ==================== BOXED LIST Y FILAS BASE ==================== */
    static inline lv_obj_t *adw_group_title_create(lv_obj_t *parent, const char *titulo)
    {
        lv_obj_t *lbl = lv_label_create(parent);
        lv_label_set_text(lbl, titulo);
        lv_obj_set_style_text_color(lbl, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_font(lbl, ADW_FONT_HEADING, 0);
        lv_obj_set_style_pad_left(lbl, 4, 0);
        lv_obj_set_style_margin_bottom(lbl, -10, 0);
        return lbl;
    }

    static inline lv_obj_t *adw_boxed_list_create(lv_obj_t *parent)
    {
        lv_obj_t *list = lv_obj_create(parent);
        lv_obj_remove_style_all(list);
        adw_set_scrollable(list, false);
        lv_obj_set_size(list, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_bg_color(list, ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(list, ADW_OPA_CARD_BG, 0);
        lv_obj_set_style_radius(list, 12, 0);
        lv_obj_set_style_clip_corner(list, true, 0);
        lv_obj_set_style_border_width(list, 1, 0);
        lv_obj_set_style_border_color(list, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(list, 40, 0);
        lv_obj_set_style_shadow_width(list, 12, 0);
        lv_obj_set_style_shadow_offset_y(list, 2, 0);
        lv_obj_set_style_shadow_color(list, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_shadow_opa(list, 55, 0);
        return list;
    }

    static inline lv_obj_t *adw_row_base_create(lv_obj_t *boxed_list, const char *titulo,
                                                const char *subtitulo, bool divisor, bool activatable)
    {
        lv_obj_t *row = lv_obj_create(boxed_list);
        lv_obj_remove_style_all(row);
        adw_set_scrollable(row, false);
        lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_min_height(row, 54, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(row, 16, 0);
        lv_obj_set_style_pad_ver(row, 10, 0);
        lv_obj_set_style_pad_column(row, 12, 0);

        if (activatable)
        {
            adw_set_clickable(row, true);
            lv_obj_set_style_bg_color(row, ADW_COLOR_FG, LV_STATE_PRESSED);
            lv_obj_set_style_bg_opa(row, 20, LV_STATE_PRESSED);
        }
        if (divisor)
        {
            lv_obj_set_style_border_width(row, 1, 0);
            lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
            lv_obj_set_style_border_color(row, ADW_COLOR_SHADE, 0);
            lv_obj_set_style_border_opa(row, ADW_OPA_SHADE, 0);
        }

        lv_obj_t *title_box = lv_obj_create(row);
        lv_obj_remove_style_all(title_box);
        adw_set_scrollable(title_box, false);
        adw_set_clickable(title_box, false);
        lv_obj_set_height(title_box, LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(title_box, 1);
        lv_obj_set_flex_flow(title_box, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(title_box, 2, 0);

        lv_obj_t *lbl_t = lv_label_create(title_box);
        lv_label_set_text(lbl_t, titulo);
        lv_obj_set_style_text_color(lbl_t, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_font(lbl_t, ADW_FONT_BODY, 0);

        if (subtitulo && subtitulo[0] != '\0')
        {
            lv_obj_t *lbl_s = lv_label_create(title_box);
            lv_label_set_text(lbl_s, subtitulo);
            lv_obj_set_style_text_color(lbl_s, ADW_COLOR_FG, 0);
            lv_obj_set_style_text_opa(lbl_s, ADW_OPA_DIM, 0);
            lv_obj_set_style_text_font(lbl_s, ADW_FONT_CAPTION, 0);
        }
        return row;
    }

    static inline lv_obj_t *adw_switch_row_create(lv_obj_t *boxed_list, const char *titulo,
                                                  const char *subtitulo, bool inicial, bool divisor)
    {
        lv_obj_t *row = adw_row_base_create(boxed_list, titulo, subtitulo, divisor, false);
        lv_obj_t *sw = lv_switch_create(row);
        lv_obj_set_size(sw, 48, 26);
        lv_obj_set_style_bg_color(sw, ADW_COLOR_FG, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(sw, ADW_OPA_BORDER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(sw, ADW_COLOR_ACCENT_BG, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_bg_color(sw, ADW_COLOR_FG, LV_PART_KNOB);
        lv_obj_set_style_pad_all(sw, -3, LV_PART_KNOB);
        if (inicial)
            lv_obj_add_state(sw, LV_STATE_CHECKED);
        return sw;
    }

    static inline void adw_check_row_click_cb(lv_event_t *e)
    {
        lv_obj_t *cb = (lv_obj_t *)lv_event_get_user_data(e);
        if (!cb || lv_obj_has_state(cb, LV_STATE_DISABLED))
            return;
        if (lv_obj_has_state(cb, LV_STATE_CHECKED))
            lv_obj_remove_state(cb, LV_STATE_CHECKED);
        else
            lv_obj_add_state(cb, LV_STATE_CHECKED);
        lv_obj_send_event(cb, LV_EVENT_VALUE_CHANGED, NULL);
    }

    static inline lv_obj_t *adw_check_row_create(lv_obj_t *boxed_list, const char *titulo,
                                                 const char *subtitulo, bool inicial, bool divisor)
    {
        lv_obj_t *row = adw_row_base_create(boxed_list, titulo, subtitulo, divisor, true);
        lv_obj_t *cb = lv_checkbox_create(row);
        lv_checkbox_set_text(cb, "");
        adw_set_clickable(cb, false); /* El clic lo maneja toda la fila */
        lv_obj_set_style_pad_column(cb, 0, 0);

        lv_obj_set_style_radius(cb, 6, LV_PART_INDICATOR);
        lv_obj_set_style_pad_all(cb, 4, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(cb, ADW_COLOR_FG, LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(cb, ADW_OPA_CTRL_BG, LV_PART_INDICATOR);
        lv_obj_set_style_border_width(cb, 2, LV_PART_INDICATOR);
        lv_obj_set_style_border_color(cb, ADW_COLOR_FG, LV_PART_INDICATOR);
        lv_obj_set_style_border_opa(cb, ADW_OPA_BORDER, LV_PART_INDICATOR);

        lv_obj_set_style_bg_color(cb, ADW_COLOR_ACCENT_BG, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_bg_opa(cb, LV_OPA_COVER, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_border_color(cb, ADW_COLOR_ACCENT_BG, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_border_opa(cb, LV_OPA_COVER, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_text_color(cb, ADW_COLOR_FG, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_text_font(cb, ADW_FONT_ICONS, LV_PART_INDICATOR | LV_STATE_CHECKED);

        if (inicial)
            lv_obj_add_state(cb, LV_STATE_CHECKED);

        lv_obj_add_event_cb(row, adw_check_row_click_cb, LV_EVENT_CLICKED, cb);
        return cb;
    }

    static inline lv_obj_t *adw_entry_row_create(lv_obj_t *boxed_list, const char *titulo,
                                                 const char *placeholder, bool password,
                                                 lv_keyboard_mode_t kb_mode, bool divisor)
    {
        lv_obj_t *row = adw_row_base_create(boxed_list, titulo, NULL, divisor, false);
        lv_obj_t *ta = lv_textarea_create(row);
        lv_textarea_set_one_line(ta, true);
        lv_textarea_set_password_mode(ta, password);
        lv_textarea_set_placeholder_text(ta, placeholder);
        lv_obj_set_width(ta, 240);
        lv_obj_set_style_radius(ta, 9, 0);
        lv_obj_set_style_bg_color(ta, ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(ta, ADW_OPA_CTRL_BG, 0);
        lv_obj_set_style_border_width(ta, 0, 0);
        lv_obj_set_style_outline_color(ta, ADW_COLOR_ACCENT_TEXT, LV_STATE_FOCUSED);
        lv_obj_set_style_outline_opa(ta, 128, LV_STATE_FOCUSED);
        lv_obj_set_style_outline_width(ta, 2, LV_STATE_FOCUSED);
        lv_obj_set_style_text_color(ta, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_font(ta, ADW_FONT_BODY, 0);
        adw_bind_keyboard(ta, kb_mode);
        return ta;
    }

    static inline lv_obj_t *adw_value_row_create(lv_obj_t *boxed_list, const char *titulo,
                                                 const char *subtitulo, const char *valor_ini,
                                                 lv_color_t color_val, const lv_font_t *font_val,
                                                 bool divisor)
    {
        lv_obj_t *row = adw_row_base_create(boxed_list, titulo, subtitulo, divisor, false);
        lv_obj_t *lbl_val = lv_label_create(row);
        lv_label_set_text(lbl_val, valor_ini ? valor_ini : "");
        lv_obj_set_style_text_color(lbl_val, color_val, 0);
        lv_obj_set_style_text_font(lbl_val, font_val ? font_val : ADW_FONT_HEADING, 0);
        return lbl_val;
    }

    static inline lv_obj_t *adw_nav_row_create(lv_obj_t *boxed_list, const char *lv_symbol_icon,
                                               const char *titulo, const char *subtitulo,
                                               const char *estado_der, bool divisor)
    {
        lv_obj_t *row = adw_row_base_create(boxed_list, titulo, subtitulo, divisor, true);

        if (lv_symbol_icon && lv_symbol_icon[0] != '\0')
        {
            lv_obj_t *ico = lv_label_create(row);
            lv_label_set_text(ico, lv_symbol_icon);
            lv_obj_set_style_text_font(ico, ADW_FONT_ICONS, 0);
            lv_obj_set_style_text_color(ico, ADW_COLOR_FG, 0);
            lv_obj_set_style_text_opa(ico, 200, 0);
            lv_obj_move_to_index(ico, 0);
        }

        if (estado_der && estado_der[0] != '\0')
        {
            lv_obj_t *lbl_est = lv_label_create(row);
            lv_label_set_text(lbl_est, estado_der);
            lv_obj_set_style_text_font(lbl_est, ADW_FONT_CAPTION, 0);
            lv_obj_set_style_text_color(lbl_est, ADW_COLOR_FG, 0);
            lv_obj_set_style_text_opa(lbl_est, ADW_OPA_DIM, 0);
        }

        lv_obj_t *arrow = lv_label_create(row);
        lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
        lv_obj_set_style_text_font(arrow, ADW_FONT_ICONS, 0);
        lv_obj_set_style_text_color(arrow, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_opa(arrow, ADW_OPA_DIM, 0);
        return row;
    }

    static inline void adw_combo_open_cb(lv_event_t *e)
    {
        lv_obj_t *dd = lv_event_get_target_obj(e);
        lv_obj_t *list = lv_dropdown_get_list(dd);
        if (!list)
            return;
        lv_obj_set_style_bg_color(list, ADW_COLOR_DIALOG_BG, 0);
        lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(list, 1, 0);
        lv_obj_set_style_border_color(list, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(list, ADW_OPA_SHADE, 0);
        lv_obj_set_style_radius(list, 12, 0);
        lv_obj_set_style_pad_all(list, 6, 0);
        lv_obj_set_style_text_color(list, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_font(list, ADW_FONT_BODY, 0);
        lv_obj_set_style_bg_color(list, ADW_COLOR_ACCENT_BG, LV_PART_SELECTED | LV_STATE_CHECKED);
        lv_obj_set_style_bg_opa(list, LV_OPA_COVER, LV_PART_SELECTED | LV_STATE_CHECKED);
        lv_obj_set_style_text_color(list, ADW_COLOR_FG, LV_PART_SELECTED | LV_STATE_CHECKED);
    }

    static inline lv_obj_t *adw_combo_row_create(lv_obj_t *boxed_list, const char *titulo,
                                                 const char *subtitulo, const char *opciones_newline,
                                                 uint32_t seleccionada, bool divisor)
    {
        lv_obj_t *row = adw_row_base_create(boxed_list, titulo, subtitulo, divisor, false);
        lv_obj_t *dd = lv_dropdown_create(row);
        lv_dropdown_set_options(dd, opciones_newline);
        lv_dropdown_set_selected(dd, seleccionada);
        lv_dropdown_set_symbol(dd, LV_SYMBOL_DOWN);
        lv_obj_set_width(dd, 200);
        lv_obj_set_style_radius(dd, 9, 0);
        lv_obj_set_style_bg_color(dd, ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(dd, ADW_OPA_CTRL_BG, 0);
        lv_obj_set_style_bg_opa(dd, ADW_OPA_CTRL_ACT, LV_STATE_PRESSED);
        lv_obj_set_style_border_width(dd, 0, 0);
        lv_obj_set_style_pad_hor(dd, 12, 0);
        lv_obj_set_style_pad_ver(dd, 8, 0);
        lv_obj_set_style_text_color(dd, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_font(dd, ADW_FONT_BODY, 0);
        lv_obj_set_style_text_font(dd, ADW_FONT_ICONS, LV_PART_INDICATOR);
        lv_obj_set_style_text_opa(dd, 200, LV_PART_INDICATOR);
        lv_obj_add_event_cb(dd, adw_combo_open_cb, LV_EVENT_READY, NULL);
        return dd;
    }

    static inline void adw_spin_dec_cb(lv_event_t *e)
    {
        lv_obj_t *sb = (lv_obj_t *)lv_event_get_user_data(e);
        lv_spinbox_decrement(sb);
    }

    static inline void adw_spin_inc_cb(lv_event_t *e)
    {
        lv_obj_t *sb = (lv_obj_t *)lv_event_get_user_data(e);
        lv_spinbox_increment(sb);
    }

    static inline lv_obj_t *adw_spin_btn_create(lv_obj_t *parent, const char *symbol,
                                                lv_event_cb_t cb, lv_obj_t *sb)
    {
        lv_obj_t *btn = lv_button_create(parent);
        lv_obj_remove_style_all(btn);
        lv_obj_set_size(btn, 34, 34);
        lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(btn, ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(btn, ADW_OPA_CTRL_BG, 0);
        lv_obj_set_style_bg_opa(btn, ADW_OPA_CTRL_ACT, LV_STATE_PRESSED);
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, sb);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, symbol);
        lv_obj_set_style_text_font(lbl, ADW_FONT_ICONS, 0);
        lv_obj_set_style_text_color(lbl, ADW_COLOR_FG, 0);
        lv_obj_center(lbl);
        return btn;
    }

    static inline lv_obj_t *adw_spin_row_create(lv_obj_t *boxed_list, const char *titulo,
                                                const char *subtitulo, int32_t min_val, int32_t max_val,
                                                int32_t inicial, uint32_t digitos, uint32_t decimales,
                                                int32_t paso, bool divisor)
    {
        lv_obj_t *row = adw_row_base_create(boxed_list, titulo, subtitulo, divisor, false);

        lv_obj_t *ctrl = lv_obj_create(row);
        lv_obj_remove_style_all(ctrl);
        adw_set_scrollable(ctrl, false);
        lv_obj_set_size(ctrl, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(ctrl, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(ctrl, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(ctrl, 8, 0);

        lv_obj_t *sb = lv_spinbox_create(ctrl);
        lv_spinbox_set_range(sb, min_val, max_val);
        lv_spinbox_set_digit_format(sb, digitos, decimales > 0 ? (digitos - decimales) : 0);
        lv_spinbox_set_step(sb, paso > 0 ? paso : 1);
        lv_spinbox_set_value(sb, inicial);
        lv_obj_set_width(sb, 88);
        lv_obj_set_style_radius(sb, 9, 0);
        lv_obj_set_style_bg_color(sb, ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(sb, ADW_OPA_CTRL_BG, 0);
        lv_obj_set_style_border_width(sb, 0, 0);
        lv_obj_set_style_pad_ver(sb, 6, 0);
        lv_obj_set_style_pad_hor(sb, 8, 0);
        lv_obj_set_style_text_align(sb, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(sb, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_font(sb, ADW_FONT_HEADING, 0);
        lv_obj_set_style_opa(sb, LV_OPA_TRANSP, LV_PART_CURSOR);

        lv_obj_t *btn_minus = adw_spin_btn_create(ctrl, LV_SYMBOL_MINUS, adw_spin_dec_cb, sb);
        lv_obj_move_to_index(btn_minus, 0);
        adw_spin_btn_create(ctrl, LV_SYMBOL_PLUS, adw_spin_inc_cb, sb);
        return sb;
    }

    static inline lv_obj_t *adw_action_card_create(lv_obj_t *parent, const char *lv_symbol_icon,
                                                   lv_color_t icon_color, const char *titulo,
                                                   const char *subtitulo, int32_t w, int32_t h)
    {
        lv_obj_t *card = lv_button_create(parent);
        lv_obj_remove_style_all(card);
        lv_obj_set_size(card, w, h);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(card, 18, 0);
        lv_obj_set_style_pad_row(card, 10, 0);
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

        if (lv_symbol_icon && lv_symbol_icon[0] != '\0')
        {
            lv_obj_t *badge = lv_obj_create(card);
            lv_obj_remove_style_all(badge);
            adw_set_clickable(badge, false);
            lv_obj_set_size(badge, 52, 52);
            lv_obj_set_style_radius(badge, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(badge, icon_color, 0);
            lv_obj_set_style_bg_opa(badge, 38, 0);

            lv_obj_t *ico = lv_label_create(badge);
            lv_label_set_text(ico, lv_symbol_icon);
            lv_obj_set_style_text_font(ico, ADW_FONT_ICONS, 0);
            lv_obj_set_style_text_color(ico, icon_color, 0);
            lv_obj_center(ico);
        }

        lv_obj_t *lbl_t = lv_label_create(card);
        lv_label_set_text(lbl_t, titulo);
        lv_obj_set_style_text_font(lbl_t, ADW_FONT_TITLE_3, 0);
        lv_obj_set_style_text_color(lbl_t, ADW_COLOR_FG, 0);

        if (subtitulo && subtitulo[0] != '\0')
        {
            lv_obj_t *lbl_s = lv_label_create(card);
            lv_label_set_text(lbl_s, subtitulo);
            lv_obj_set_width(lbl_s, lv_pct(100));
            lv_label_set_long_mode(lbl_s, LV_LABEL_LONG_WRAP);
            lv_obj_set_style_text_align(lbl_s, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_font(lbl_s, ADW_FONT_CAPTION, 0);
            lv_obj_set_style_text_color(lbl_s, ADW_COLOR_FG, 0);
            lv_obj_set_style_text_opa(lbl_s, ADW_OPA_DIM, 0);
        }
        return card;
    }

    static inline void adw_numpad_digit_cb(lv_event_t *e)
    {
        lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
        if (!ta)
            return;
        lv_obj_t *btn = lv_event_get_target_obj(e);
        lv_obj_t *lbl = lv_obj_get_child(btn, 0);
        if (!lbl)
            return;
        const char *txt = lv_label_get_text(lbl);
        if (txt[0] == '.' && strchr(lv_textarea_get_text(ta), '.') != NULL)
            return;
        lv_textarea_add_text(ta, txt);
    }

    static inline void adw_numpad_clear_cb(lv_event_t *e)
    {
        lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
        if (ta)
            lv_textarea_set_text(ta, "");
    }

    static inline void adw_numpad_back_cb(lv_event_t *e)
    {
        lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
        if (ta)
            lv_textarea_delete_char(ta);
    }

    static inline void adw_numpad_ok_cb(lv_event_t *e)
    {
        lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
        if (ta)
            lv_obj_send_event(ta, LV_EVENT_READY, NULL);
    }

    static inline lv_obj_t *adw_numpad_key_create(lv_obj_t *row, const char *txt,
                                                  const lv_font_t *font, adw_btn_variant_t var,
                                                  lv_event_cb_t cb, lv_obj_t *target_ta)
    {
        lv_obj_t *btn = adw_button_create(row, txt, var, false);
        lv_obj_set_height(btn, 48);
        lv_obj_set_flex_grow(btn, 1);
        lv_obj_set_style_radius(btn, 10, 0);
        lv_obj_set_style_pad_all(btn, 0, 0);
        lv_obj_t *lbl = lv_obj_get_child(btn, 0);
        if (lbl && font)
            lv_obj_set_style_text_font(lbl, font, 0);
        if (cb)
            lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, target_ta);
        return btn;
    }

    static inline lv_obj_t *adw_numpad_create(lv_obj_t *parent, lv_obj_t *target_ta,
                                              bool allow_decimal, const char *confirm_text)
    {
        lv_obj_t *pad = lv_obj_create(parent);
        lv_obj_remove_style_all(pad);
        adw_set_scrollable(pad, false);
        lv_obj_set_size(pad, 290, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(pad, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(pad, 8, 0);

        static const char *const digits[3][3] = {
            {"1", "2", "3"},
            {"4", "5", "6"},
            {"7", "8", "9"}};

        for (int r = 0; r < 3; r++)
        {
            lv_obj_t *row = lv_obj_create(pad);
            lv_obj_remove_style_all(row);
            adw_set_scrollable(row, false);
            lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
            lv_obj_set_style_pad_column(row, 8, 0);
            for (int c = 0; c < 3; c++)
            {
                adw_numpad_key_create(row, digits[r][c], ADW_FONT_TITLE_3, ADW_BTN_DEFAULT,
                                      adw_numpad_digit_cb, target_ta);
            }
        }

        lv_obj_t *row4 = lv_obj_create(pad);
        lv_obj_remove_style_all(row4);
        adw_set_scrollable(row4, false);
        lv_obj_set_size(row4, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(row4, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_column(row4, 8, 0);

        if (allow_decimal)
        {
            adw_numpad_key_create(row4, ".", ADW_FONT_TITLE_3, ADW_BTN_DEFAULT,
                                  adw_numpad_digit_cb, target_ta);
        }
        else
        {
            adw_numpad_key_create(row4, "C", ADW_FONT_HEADING, ADW_BTN_DESTRUCTIVE,
                                  adw_numpad_clear_cb, target_ta);
        }
        adw_numpad_key_create(row4, "0", ADW_FONT_TITLE_3, ADW_BTN_DEFAULT,
                              adw_numpad_digit_cb, target_ta);
        adw_numpad_key_create(row4, LV_SYMBOL_BACKSPACE, ADW_FONT_ICONS, ADW_BTN_DEFAULT,
                              adw_numpad_back_cb, target_ta);

        if (confirm_text && confirm_text[0] != '\0')
        {
            lv_obj_t *btn_ok = adw_button_create(pad, confirm_text, ADW_BTN_SUGGESTED, true);
            lv_obj_set_width(btn_ok, lv_pct(100));
            lv_obj_set_height(btn_ok, 44);
            lv_obj_set_style_margin_top(btn_ok, 4, 0);
            lv_obj_add_event_cb(btn_ok, adw_numpad_ok_cb, LV_EVENT_CLICKED, target_ta);
        }
        return pad;
    }

    static inline lv_obj_t *adw_qr_card_create(lv_obj_t *parent, const char *data,
                                               int32_t qr_size, const char *pairing_code,
                                               lv_obj_t **out_qr)
    {
        lv_obj_t *card = adw_boxed_list_create(parent);
        lv_obj_set_size(card, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(card, 20, 0);
        lv_obj_set_style_pad_row(card, 14, 0);

        lv_obj_t *quiet_zone = lv_obj_create(card);
        lv_obj_remove_style_all(quiet_zone);
        adw_set_scrollable(quiet_zone, false);
        lv_obj_set_size(quiet_zone, qr_size + 24, qr_size + 24);
        lv_obj_set_style_bg_color(quiet_zone, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(quiet_zone, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(quiet_zone, 12, 0);

#if LV_USE_QRCODE
        lv_obj_t *qr = lv_qrcode_create(quiet_zone);
        lv_qrcode_set_size(qr, qr_size);
        lv_qrcode_set_dark_color(qr, lv_color_black());
        lv_qrcode_set_light_color(qr, lv_color_white());
        if (data && data[0] != '\0')
        {
            lv_qrcode_update(qr, data, strlen(data));
        }
        lv_obj_center(qr);
        if (out_qr)
            *out_qr = qr;
#else
    lv_obj_t *ph = lv_label_create(quiet_zone);
    lv_label_set_text(ph, "QR\n(Activar\nLV_USE_QRCODE)");
    lv_obj_set_style_text_align(ph, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(ph, ADW_COLOR_WINDOW_BG, 0);
    lv_obj_set_style_text_font(ph, ADW_FONT_CAPTION, 0);
    lv_obj_center(ph);
    if (out_qr)
        *out_qr = ph;
#endif

        if (pairing_code && pairing_code[0] != '\0')
        {
            lv_obj_t *lbl_code = lv_label_create(card);
            lv_label_set_text(lbl_code, pairing_code);
            lv_obj_set_style_text_font(lbl_code, ADW_FONT_TITLE_2, 0);
            lv_obj_set_style_text_color(lbl_code, ADW_COLOR_ACCENT_TEXT, 0);
        }
        return card;
    }

    /* ==================== ALERT DIALOG ==================== */
    ADW_GLOBAL lv_obj_t *adw_active_dialog = NULL;
    ADW_GLOBAL adw_dialog_cb_t adw_dialog_confirm_cb = NULL;
    ADW_GLOBAL void *adw_dialog_user_data = NULL;

    static inline void adw_alert_dialog_close(void)
    {
        if (adw_active_dialog)
        {
            lv_obj_delete(adw_active_dialog);
            adw_active_dialog = NULL;
        }
        adw_dialog_confirm_cb = NULL;
        adw_dialog_user_data = NULL;
    }

    static inline void adw_dialog_cancel_btn_cb(lv_event_t *e)
    {
        LV_UNUSED(e);
        adw_alert_dialog_close();
    }

    static inline void adw_dialog_confirm_btn_cb(lv_event_t *e)
    {
        LV_UNUSED(e);
        adw_dialog_cb_t cb = adw_dialog_confirm_cb;
        void *ud = adw_dialog_user_data;
        adw_alert_dialog_close();
        if (cb)
            cb(ud);
    }

    static inline lv_obj_t *adw_alert_dialog_show(const char *titulo, const char *cuerpo,
                                                  const char *cancel_txt, const char *confirm_txt,
                                                  adw_btn_variant_t confirm_variant,
                                                  adw_dialog_cb_t on_confirm, void *user_data)
    {
        adw_alert_dialog_close();
        adw_dialog_confirm_cb = on_confirm;
        adw_dialog_user_data = user_data;

        lv_obj_t *dim = lv_obj_create(lv_layer_top());
        lv_obj_remove_style_all(dim);
        lv_obj_set_size(dim, lv_pct(100), lv_pct(100));
        lv_obj_set_style_bg_color(dim, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_bg_opa(dim, 160, 0);
        adw_set_clickable(dim, true);
        adw_active_dialog = dim;

        lv_obj_t *sheet = lv_obj_create(dim);
        lv_obj_remove_style_all(sheet);
        adw_set_scrollable(sheet, false);
        lv_obj_set_size(sheet, 410, LV_SIZE_CONTENT);
        lv_obj_center(sheet);
        lv_obj_set_flex_flow(sheet, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(sheet, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_bg_color(sheet, ADW_COLOR_DIALOG_BG, 0);
        lv_obj_set_style_bg_opa(sheet, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(sheet, 18, 0);
        lv_obj_set_style_pad_all(sheet, 24, 0);
        lv_obj_set_style_pad_row(sheet, 12, 0);
        lv_obj_set_style_border_width(sheet, 1, 0);
        lv_obj_set_style_border_color(sheet, ADW_COLOR_FG, 0);
        lv_obj_set_style_border_opa(sheet, 20, 0);
        lv_obj_set_style_shadow_width(sheet, 24, 0);
        lv_obj_set_style_shadow_color(sheet, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_shadow_opa(sheet, 120, 0);

        lv_obj_t *lbl_t = lv_label_create(sheet);
        lv_label_set_text(lbl_t, titulo);
        lv_obj_set_width(lbl_t, lv_pct(100));
        lv_obj_set_style_text_align(lbl_t, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(lbl_t, ADW_FONT_TITLE_2, 0);
        lv_obj_set_style_text_color(lbl_t, ADW_COLOR_FG, 0);

        if (cuerpo && cuerpo[0] != '\0')
        {
            lv_obj_t *lbl_b = lv_label_create(sheet);
            lv_label_set_text(lbl_b, cuerpo);
            lv_obj_set_width(lbl_b, lv_pct(100));
            lv_label_set_long_mode(lbl_b, LV_LABEL_LONG_WRAP);
            lv_obj_set_style_text_align(lbl_b, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_font(lbl_b, ADW_FONT_BODY, 0);
            lv_obj_set_style_text_color(lbl_b, ADW_COLOR_FG, 0);
            lv_obj_set_style_text_opa(lbl_b, 215, 0);
        }

        lv_obj_t *actions = lv_obj_create(sheet);
        lv_obj_remove_style_all(actions);
        adw_set_scrollable(actions, false);
        lv_obj_set_size(actions, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_margin_top(actions, 10, 0);
        lv_obj_set_flex_flow(actions, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_column(actions, 12, 0);

        if (cancel_txt && cancel_txt[0] != '\0')
        {
            lv_obj_t *btn_c = adw_button_create(actions, cancel_txt, ADW_BTN_DEFAULT, false);
            lv_obj_set_flex_grow(btn_c, 1);
            lv_obj_set_height(btn_c, 42);
            lv_obj_set_style_radius(btn_c, 12, 0);
            lv_obj_add_event_cb(btn_c, adw_dialog_cancel_btn_cb, LV_EVENT_CLICKED, NULL);
        }

        if (confirm_txt && confirm_txt[0] != '\0')
        {
            lv_obj_t *btn_ok = adw_button_create(actions, confirm_txt, confirm_variant, false);
            lv_obj_set_flex_grow(btn_ok, 1);
            lv_obj_set_height(btn_ok, 42);
            lv_obj_set_style_radius(btn_ok, 12, 0);
            lv_obj_add_event_cb(btn_ok, adw_dialog_confirm_btn_cb, LV_EVENT_CLICKED, NULL);
        }
        return dim;
    }

    /* ==================== TOAST ANIMADO ==================== */
    ADW_GLOBAL lv_obj_t *adw_active_toast = NULL;
    ADW_GLOBAL lv_timer_t *adw_toast_timer = NULL;

    static inline void adw_toast_anim_cb(void *var, int32_t v)
    {
        lv_obj_t *toast = (lv_obj_t *)var;
        if (v < 0)
            v = 0;
        if (v > 255)
            v = 255;
        lv_obj_set_style_opa(toast, (lv_opa_t)v, 0);
        lv_obj_set_style_translate_y(toast, ((255 - v) * 28) / 255, 0);
    }

    static inline void adw_toast_out_done_cb(lv_anim_t *a)
    {
        lv_obj_t *toast = (lv_obj_t *)a->var;
        if (toast == adw_active_toast)
        {
            adw_active_toast = NULL;
        }
        lv_obj_delete(toast);
    }

    static inline void adw_toast_close_cb(lv_timer_t *t)
    {
        LV_UNUSED(t);
        adw_toast_timer = NULL;
        if (!adw_active_toast)
            return;

        lv_anim_delete(adw_active_toast, NULL);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, adw_active_toast);
        lv_anim_set_values(&a, 255, 0);
        lv_anim_set_duration(&a, 180);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
        lv_anim_set_exec_cb(&a, adw_toast_anim_cb);
        lv_anim_set_completed_cb(&a, adw_toast_out_done_cb);
        lv_anim_start(&a);
    }

    static inline void adw_toast_show(const char *mensaje, uint32_t ms)
    {
        if (adw_toast_timer)
        {
            lv_timer_delete(adw_toast_timer);
            adw_toast_timer = NULL;
        }
        if (adw_active_toast)
        {
            lv_anim_delete(adw_active_toast, NULL);
            lv_obj_delete(adw_active_toast);
            adw_active_toast = NULL;
        }

        adw_active_toast = lv_obj_create(lv_layer_top());
        lv_obj_remove_style_all(adw_active_toast);
        adw_set_scrollable(adw_active_toast, false);
        lv_obj_set_size(adw_active_toast, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_align(adw_active_toast, LV_ALIGN_BOTTOM_MID, 0, -24);
        lv_obj_set_style_bg_color(adw_active_toast, ADW_COLOR_TOAST_BG, 0);
        lv_obj_set_style_bg_opa(adw_active_toast, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(adw_active_toast, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_pad_hor(adw_active_toast, 22, 0);
        lv_obj_set_style_pad_ver(adw_active_toast, 10, 0);
        lv_obj_set_style_shadow_width(adw_active_toast, 14, 0);
        lv_obj_set_style_shadow_color(adw_active_toast, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_shadow_opa(adw_active_toast, 90, 0);

        lv_obj_t *lbl = lv_label_create(adw_active_toast);
        lv_label_set_text(lbl, mensaje);
        lv_obj_set_style_text_color(lbl, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_font(lbl, ADW_FONT_BODY, 0);

        adw_toast_anim_cb(adw_active_toast, 0);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, adw_active_toast);
        lv_anim_set_values(&a, 0, 255);
        lv_anim_set_duration(&a, 220);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_set_exec_cb(&a, adw_toast_anim_cb);
        lv_anim_start(&a);

        adw_toast_timer = lv_timer_create(adw_toast_close_cb, ms, NULL);
        lv_timer_set_repeat_count(adw_toast_timer, 1);
    }

    /* ==================== CORTINILLA DE CARGA ==================== */
    static inline lv_obj_t *adw_loading_overlay_create(const char *texto)
    {
        lv_obj_t *overlay = lv_obj_create(lv_layer_top());
        lv_obj_remove_style_all(overlay);
        lv_obj_set_size(overlay, lv_pct(100), lv_pct(100));
        lv_obj_set_style_bg_color(overlay, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_bg_opa(overlay, 165, 0);
        lv_obj_set_flex_flow(overlay, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(overlay, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_row(overlay, 16, 0);
        adw_set_clickable(overlay, true);

        lv_obj_t *spin = lv_spinner_create(overlay);
        lv_spinner_set_anim_params(spin, 1000, 270);
        lv_obj_set_size(spin, 56, 56);
        lv_obj_set_style_arc_color(spin, ADW_COLOR_ACCENT_TEXT, LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(spin, 5, LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(spin, 5, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(spin, 40, LV_PART_MAIN);

        if (texto)
        {
            lv_obj_t *lbl = lv_label_create(overlay);
            lv_label_set_text(lbl, texto);
            lv_obj_set_style_text_color(lbl, ADW_COLOR_FG, 0);
            lv_obj_set_style_text_font(lbl, ADW_FONT_TITLE_3, 0);
        }
        return overlay;
    }

#ifdef __cplusplus
} // extern "C"

/* ==================== WRAPPER C++20 ESTILO GTKMM4 ==================== */
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace adw
{

    struct Str
    {
        const char *ptr = "";
        constexpr Str(const char *s = "") noexcept : ptr(s ? s : "") {}
        Str(const std::string &s) noexcept : ptr(s.c_str()) {}
        constexpr operator const char *() const noexcept { return ptr; }
        constexpr bool empty() const noexcept { return ptr[0] == '\0'; }
    };

    enum class Orientation
    {
        Horizontal,
        Vertical
    };

    enum class BtnVariant
    {
        Default = ADW_BTN_DEFAULT,
        Suggested = ADW_BTN_SUGGESTED,
        Destructive = ADW_BTN_DESTRUCTIVE,
        Flat = ADW_BTN_FLAT,
        Warning = ADW_BTN_WARNING
    };

    enum class KeyboardMode
    {
        TextLower = LV_KEYBOARD_MODE_TEXT_LOWER,
        TextUpper = LV_KEYBOARD_MODE_TEXT_UPPER,
        Special = LV_KEYBOARD_MODE_SPECIAL,
        Number = LV_KEYBOARD_MODE_NUMBER
    };

    class UiDispatcher
    {
    public:
        static void init(uint32_t period_ms = 16)
        {
            if (timer_)
                return;
            timer_ = lv_timer_create([](lv_timer_t *)
                                     { drain(); }, period_ms, nullptr);
        }

        static void post(std::function<void()> task)
        {
            std::lock_guard<std::mutex> lock(mtx_);
            queue_.push_back(std::move(task));
        }

        static void drain()
        {
            std::vector<std::function<void()>> pending;
            {
                std::lock_guard<std::mutex> lock(mtx_);
                if (queue_.empty())
                    return;
                pending.swap(queue_);
            }
            for (auto &fn : pending)
            {
                if (fn)
                    fn();
            }
        }

    private:
        inline static std::mutex mtx_;
        inline static std::vector<std::function<void()>> queue_;
        inline static lv_timer_t *timer_ = nullptr;
    };

    inline void post_to_ui(std::function<void()> task)
    {
        UiDispatcher::post(std::move(task));
    }

    namespace detail
    {

        struct EventSlot
        {
            std::function<void(lv_event_t *)> cb;
        };

        inline void event_trampoline(lv_event_t *e)
        {
            auto *slot = static_cast<EventSlot *>(lv_event_get_user_data(e));
            if (slot && slot->cb)
                slot->cb(e);
        }

        inline void event_delete_cb(lv_event_t *e)
        {
            auto *slot = static_cast<EventSlot *>(lv_event_get_user_data(e));
            delete slot;
        }

        inline void connect_raw(lv_obj_t *obj, lv_event_code_t code, std::function<void(lv_event_t *)> cb)
        {
            if (!obj)
                return;
            auto *slot = new EventSlot{std::move(cb)};
            lv_obj_add_event_cb(obj, event_trampoline, code, slot);
            lv_obj_add_event_cb(obj, event_delete_cb, LV_EVENT_DELETE, slot);
        }

        inline std::function<void()> g_alert_confirm_cb;

        inline void alert_confirm_trampoline(void *)
        {
            auto fn = std::move(g_alert_confirm_cb);
            g_alert_confirm_cb = nullptr;
            if (fn)
                fn();
        }

    } // namespace detail

    template <typename... Args>
    class SignalProxy
    {
    public:
        using Adapter = std::function<void(std::function<void(Args...)>)>;
        explicit SignalProxy(Adapter adapter) : adapter_(std::move(adapter)) {}

        void connect(std::function<void(Args...)> cb) const
        {
            if (adapter_)
                adapter_(std::move(cb));
        }

    private:
        Adapter adapter_;
    };

    class Timer
    {
    public:
        Timer() = default;
        ~Timer() { stop(); }

        Timer(const Timer &) = delete;
        Timer &operator=(const Timer &) = delete;
        Timer(Timer &&other) noexcept : state_(std::move(other.state_)) {}
        Timer &operator=(Timer &&other) noexcept
        {
            if (this != &other)
            {
                stop();
                state_ = std::move(other.state_);
            }
            return *this;
        }

        void start(uint32_t period_ms, std::function<bool()> cb)
        {
            stop();
            state_ = std::make_unique<State>();
            state_->cb = std::move(cb);
            state_->timer = lv_timer_create(
                [](lv_timer_t *t)
                {
                    auto *st = static_cast<State *>(lv_timer_get_user_data(t));
                    if (!st || !st->cb || !st->cb())
                    {
                        if (st)
                            st->timer = nullptr;
                        lv_timer_delete(t);
                    }
                },
                period_ms, state_.get());
        }

        void stop()
        {
            if (state_ && state_->timer)
            {
                lv_timer_delete(state_->timer);
                state_->timer = nullptr;
            }
            state_.reset();
        }

    private:
        struct State
        {
            lv_timer_t *timer = nullptr;
            std::function<bool()> cb;
        };
        std::unique_ptr<State> state_;
    };

    class Widget
    {
    public:
        Widget() = default;
        explicit Widget(lv_obj_t *obj) noexcept : obj_(obj) {}
        Widget(const Widget &) = default;
        Widget &operator=(const Widget &) = default;
        Widget(Widget &&) noexcept = default;
        Widget &operator=(Widget &&) noexcept = default;
        virtual ~Widget() = default;

        lv_obj_t *raw() const noexcept { return obj_; }
        explicit operator bool() const noexcept { return obj_ != nullptr; }

        virtual void set_visible(bool visible)
        {
            if (auto *target = layout_raw())
                adw_set_hidden(target, !visible);
        }
        void show() { set_visible(true); }
        void hide() { set_visible(false); }

        bool get_visible() const
        {
            auto *target = layout_raw();
            return target && !adw_is_hidden(target);
        }

        virtual void set_sensitive(bool sensitive)
        {
            if (!obj_)
                return;
            if (sensitive)
            {
                lv_obj_remove_state(obj_, LV_STATE_DISABLED);
                adw_set_clickable(obj_, true);
                lv_obj_set_style_opa(layout_raw(), LV_OPA_COVER, 0);
            }
            else
            {
                lv_obj_add_state(obj_, LV_STATE_DISABLED);
                adw_set_clickable(obj_, false);
                lv_obj_set_style_opa(layout_raw(), ADW_OPA_DIM, 0);
            }
        }

        void set_size_request(int32_t w, int32_t h)
        {
            if (auto *t = layout_raw())
                lv_obj_set_size(t, w, h);
        }
        void set_width(int32_t w)
        {
            if (auto *t = layout_raw())
                lv_obj_set_width(t, w);
        }
        void set_height(int32_t h)
        {
            if (auto *t = layout_raw())
                lv_obj_set_height(t, h);
        }
        void set_hexpand(bool expand = true)
        {
            if (auto *t = layout_raw())
            {
                lv_obj_set_width(t, expand ? lv_pct(100) : LV_SIZE_CONTENT);
            }
        }
        void set_flex_grow(uint8_t grow = 1)
        {
            if (auto *t = layout_raw())
                lv_obj_set_flex_grow(t, grow);
        }

        void set_margin(int32_t px)
        {
            if (auto *t = layout_raw())
                lv_obj_set_style_margin_all(t, px, 0);
        }
        void set_margin_top(int32_t px)
        {
            if (auto *t = layout_raw())
                lv_obj_set_style_margin_top(t, px, 0);
        }
        void set_margin_bottom(int32_t px)
        {
            if (auto *t = layout_raw())
                lv_obj_set_style_margin_bottom(t, px, 0);
        }
        void set_margin_start(int32_t px)
        {
            if (auto *t = layout_raw())
                lv_obj_set_style_margin_left(t, px, 0);
        }
        void set_margin_end(int32_t px)
        {
            if (auto *t = layout_raw())
                lv_obj_set_style_margin_right(t, px, 0);
        }

        void append(Widget &child)
        {
            if (obj_ && child.layout_raw())
                lv_obj_set_parent(child.layout_raw(), obj_);
        }
        void clear_children()
        {
            if (obj_)
                lv_obj_clean(obj_);
        }
        void remove()
        {
            if (auto *t = layout_raw())
            {
                lv_obj_delete(t);
                obj_ = nullptr;
            }
        }

        SignalProxy<> signal_clicked()
        {
            lv_obj_t *target = obj_;
            return SignalProxy<>([target](std::function<void()> cb)
                                 { detail::connect_raw(target, LV_EVENT_CLICKED, [fn = std::move(cb)](lv_event_t *)
                                                       {
                if (fn) fn(); }); });
        }

    protected:
        virtual lv_obj_t *layout_raw() const noexcept { return obj_; }
        lv_obj_t *obj_ = nullptr;
    };

    class Screen : public Widget
    {
    public:
        explicit Screen(lv_obj_t *scr = lv_screen_active())
            : Widget(adw_screen_prepare(scr)) {}

        lv_obj_t *init_keyboard() { return adw_keyboard_init(obj_); }
    };

    class Box : public Widget
    {
    public:
        Box() = default;
        Box(Widget &parent, Orientation orientation = Orientation::Vertical, int32_t spacing = 12)
            : Box(parent.raw(), orientation, spacing) {}

        Box(lv_obj_t *parent, Orientation orientation = Orientation::Vertical, int32_t spacing = 12)
        {
            obj_ = lv_obj_create(parent);
            lv_obj_remove_style_all(obj_);
            adw_set_scrollable(obj_, false);
            lv_obj_set_size(obj_, lv_pct(100), LV_SIZE_CONTENT);

            if (orientation == Orientation::Horizontal)
            {
                lv_obj_set_flex_flow(obj_, LV_FLEX_FLOW_ROW);
                lv_obj_set_style_pad_column(obj_, spacing, 0);
            }
            else
            {
                lv_obj_set_flex_flow(obj_, LV_FLEX_FLOW_COLUMN);
                lv_obj_set_style_pad_row(obj_, spacing, 0);
            }
        }

        void set_align(lv_flex_align_t main_a, lv_flex_align_t cross_a, lv_flex_align_t track_a)
        {
            if (obj_)
                lv_obj_set_flex_align(obj_, main_a, cross_a, track_a);
        }
        void set_padding(int32_t px)
        {
            if (obj_)
                lv_obj_set_style_pad_all(obj_, px, 0);
        }
    };

    class ScrollView : public Widget
    {
    public:
        ScrollView() = default;
        explicit ScrollView(Widget &parent, int32_t padding = 0)
        {
            obj_ = lv_obj_create(parent.raw());
            lv_obj_remove_style_all(obj_);
            lv_obj_set_width(obj_, lv_pct(100));
            lv_obj_set_flex_grow(obj_, 1);
            lv_obj_set_flex_flow(obj_, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_style_pad_all(obj_, padding, 0);
            lv_obj_set_style_pad_row(obj_, 12, 0);
            adw_set_scrollable(obj_, true);
        }
    };

    class Stack : public Widget
    {
    public:
        Stack() = default;
        explicit Stack(Widget &parent)
        {
            obj_ = lv_obj_create(parent.raw());
            lv_obj_remove_style_all(obj_);
            adw_set_scrollable(obj_, false);
            lv_obj_set_width(obj_, lv_pct(100));
            lv_obj_set_flex_grow(obj_, 1);
        }

        Box add_page(const std::string &name, int32_t spacing = 12)
        {
            Box page(*this, Orientation::Vertical, spacing);
            lv_obj_set_size(page.raw(), lv_pct(100), lv_pct(100));
            bool first = pages_.empty();
            page.set_visible(first);
            if (first)
                current_ = name;
            pages_.push_back({name, page.raw()});
            return page;
        }

        void set_visible_child(const std::string &name)
        {
            for (const auto &p : pages_)
            {
                adw_set_hidden(p.obj, p.name != name);
            }
            current_ = name;
            if (on_page_shown_)
                on_page_shown_(name);
        }

        const std::string &get_visible_child_name() const noexcept { return current_; }

        void on_page_shown(std::function<void(const std::string &)> cb)
        {
            on_page_shown_ = std::move(cb);
        }

    private:
        struct PageEntry
        {
            std::string name;
            lv_obj_t *obj;
        };
        std::vector<PageEntry> pages_;
        std::string current_;
        std::function<void(const std::string &)> on_page_shown_;
    };
    class HeaderBar : public Widget
    {
    public:
        HeaderBar() = default;
        HeaderBar(Widget &parent, Str titulo, bool show_back = true)
        {
            lv_obj_t *back_ptr = nullptr;
            obj_ = adw_headerbar_create(parent.raw(), titulo, show_back ? &back_ptr : nullptr);
            btn_back_ = back_ptr;
            // Guardar lbl_title_ ANTES de crear start_box_ y end_box_
            lbl_title_ = lv_obj_get_child(obj_, -1);

            start_box_ = make_side_box(LV_ALIGN_LEFT_MID);
            end_box_ = make_side_box(LV_ALIGN_RIGHT_MID);

            // Si hay botón de regreso, moverlo al contenedor izquierdo para que no se encime
            if (btn_back_)
            {
                lv_obj_set_parent(btn_back_, start_box_);
            }
        }

        void set_title(Str titulo)
        {
            if (lbl_title_)
                lv_label_set_text(lbl_title_, titulo);
        }

        void pack_start(Widget &child)
        {
            if (!start_box_ || !child.raw())
                return;
            lv_obj_set_parent(child.raw(), start_box_);
        }

        void pack_end(Widget &child)
        {
            if (!end_box_ || !child.raw())
                return;
            lv_obj_set_parent(child.raw(), end_box_);
        }

        SignalProxy<> signal_back_clicked()
        {
            lv_obj_t *target = btn_back_;
            return SignalProxy<>([target](std::function<void()> cb)
                                 { detail::connect_raw(target, LV_EVENT_CLICKED, [fn = std::move(cb)](lv_event_t *)
                                                       {
                if (fn) fn(); }); });
        }

    private:
        lv_obj_t *make_side_box(lv_align_t align)
        {
            lv_obj_t *box = lv_obj_create(obj_);
            lv_obj_remove_style_all(box);
            adw_set_scrollable(box, false);
            lv_obj_set_size(box, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(box, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(box, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_style_pad_column(box, 8, 0);
            lv_obj_align(box, align, 0, 0);
            return box;
        }

        lv_obj_t *btn_back_ = nullptr;
        lv_obj_t *lbl_title_ = nullptr;
        lv_obj_t *start_box_ = nullptr;
        lv_obj_t *end_box_ = nullptr;
    };
    class Page : public Widget
    {
    public:
        Page() = default;
        explicit Page(Widget &parent, int32_t max_width = 620)
            : Widget(adw_page_create(parent.raw(), max_width)) {}
    };

    class BoxedList : public Widget
    {
    public:
        BoxedList() = default;
        explicit BoxedList(Widget &parent, Str group_title = "")
        {
            if (!group_title.empty())
            {
                title_lbl_ = adw_group_title_create(parent.raw(), group_title);
            }
            obj_ = adw_boxed_list_create(parent.raw());
        }

        void set_visible(bool visible) override
        {
            if (title_lbl_)
                adw_set_hidden(title_lbl_, !visible);
            Widget::set_visible(visible);
        }

    private:
        lv_obj_t *title_lbl_ = nullptr;
    };

    class Label : public Widget
    {
    public:
        Label() = default;
        Label(Widget &parent, Str texto = "", const lv_font_t *font = ADW_FONT_BODY,
              lv_color_t color = ADW_COLOR_FG)
        {
            obj_ = lv_label_create(parent.raw());
            lv_label_set_text(obj_, texto);
            lv_obj_set_style_text_font(obj_, font, 0);
            lv_obj_set_style_text_color(obj_, color, 0);
        }

        void set_text(Str texto)
        {
            if (obj_)
                lv_label_set_text(obj_, texto);
        }
        std::string get_text() const
        {
            return obj_ ? std::string(lv_label_get_text(obj_)) : "";
        }
        void set_color(lv_color_t color, lv_opa_t opa = LV_OPA_COVER)
        {
            if (!obj_)
                return;
            lv_obj_set_style_text_color(obj_, color, 0);
            lv_obj_set_style_text_opa(obj_, opa, 0);
        }
        void set_font(const lv_font_t *font)
        {
            if (obj_ && font)
                lv_obj_set_style_text_font(obj_, font, 0);
        }
        void set_wrap(bool wrap = true)
        {
            if (obj_)
                lv_label_set_long_mode(obj_, wrap ? LV_LABEL_LONG_WRAP : LV_LABEL_LONG_CLIP);
        }
    };

    class Button : public Widget
    {
    public:
        Button() = default;
        Button(Widget &parent, Str texto, BtnVariant variant = BtnVariant::Default, bool pill = false)
            : Widget(adw_button_create(parent.raw(), texto,
                                       static_cast<adw_btn_variant_t>(variant), pill)) {}

        void set_label(Str texto)
        {
            if (!obj_)
                return;
            if (auto *lbl = lv_obj_get_child(obj_, 0))
                lv_label_set_text(lbl, texto);
        }
    };

    class IconButton : public Widget
    {
    public:
        IconButton() = default;
        IconButton(Widget &parent, Str symbol, BtnVariant variant = BtnVariant::Flat)
            : Widget(adw_icon_button_create(parent.raw(), symbol,
                                            static_cast<adw_btn_variant_t>(variant))) {}
        void set_icon(Str icon)
        {
            if (!obj_)
                return;
            if (lv_obj_t *lbl = lv_obj_get_child(obj_, 0))
            {
                lv_label_set_text(lbl, icon);
            }
        }
    };

    class SwitchRow : public Widget
    {
    public:
        SwitchRow() = default;
        SwitchRow(BoxedList &list, Str titulo, Str subtitulo = "",
                  bool inicial = false, bool divisor = true)
        {
            obj_ = adw_switch_row_create(list.raw(), titulo, subtitulo, inicial, divisor);
            row_ = lv_obj_get_parent(obj_);
        }

        bool get_active() const
        {
            return obj_ && lv_obj_has_state(obj_, LV_STATE_CHECKED);
        }
        void set_active(bool active)
        {
            if (!obj_)
                return;
            if (active)
                lv_obj_add_state(obj_, LV_STATE_CHECKED);
            else
                lv_obj_remove_state(obj_, LV_STATE_CHECKED);
        }

        SignalProxy<bool> signal_toggled()
        {
            lv_obj_t *sw = obj_;
            return SignalProxy<bool>([sw](std::function<void(bool)> cb)
                                     { detail::connect_raw(sw, LV_EVENT_VALUE_CHANGED, [sw, fn = std::move(cb)](lv_event_t *)
                                                           {
                if (fn) fn(lv_obj_has_state(sw, LV_STATE_CHECKED)); }); });
        }

    protected:
        lv_obj_t *layout_raw() const noexcept override { return row_ ? row_ : obj_; }

    private:
        lv_obj_t *row_ = nullptr;
    };

    class CheckRow : public Widget
    {
    public:
        CheckRow() = default;
        CheckRow(BoxedList &list, Str titulo, Str subtitulo = "",
                 bool inicial = true, bool divisor = true)
        {
            obj_ = adw_check_row_create(list.raw(), titulo, subtitulo, inicial, divisor);
            row_ = lv_obj_get_parent(obj_);
        }

        bool get_active() const
        {
            return obj_ && lv_obj_has_state(obj_, LV_STATE_CHECKED);
        }

        void set_active(bool active)
        {
            if (!obj_)
                return;
            if (active)
                lv_obj_add_state(obj_, LV_STATE_CHECKED);
            else
                lv_obj_remove_state(obj_, LV_STATE_CHECKED);
        }

        void set_sensitive(bool sensitive) override
        {
            if (!obj_ || !row_)
                return;
            if (sensitive)
            {
                lv_obj_remove_state(obj_, LV_STATE_DISABLED);
                adw_set_clickable(row_, true);
                lv_obj_set_style_opa(row_, LV_OPA_COVER, 0);
            }
            else
            {
                lv_obj_add_state(obj_, LV_STATE_DISABLED);
                adw_set_clickable(row_, false);
                lv_obj_set_style_opa(row_, ADW_OPA_DIM, 0);
            }
        }

        SignalProxy<bool> signal_toggled()
        {
            lv_obj_t *cb = obj_;
            return SignalProxy<bool>([cb](std::function<void(bool)> cb_fn)
                                     { detail::connect_raw(cb, LV_EVENT_VALUE_CHANGED, [cb, fn = std::move(cb_fn)](lv_event_t *)
                                                           {
                if (fn) fn(lv_obj_has_state(cb, LV_STATE_CHECKED)); }); });
        }

    protected:
        lv_obj_t *layout_raw() const noexcept override { return row_ ? row_ : obj_; }

    private:
        lv_obj_t *row_ = nullptr;
    };

    class EntryRow : public Widget
    {
    public:
        EntryRow() = default;
        EntryRow(BoxedList &list, Str titulo, Str placeholder = "", bool password = false,
                 KeyboardMode kb_mode = KeyboardMode::TextLower, bool divisor = true)
        {
            obj_ = adw_entry_row_create(list.raw(), titulo, placeholder, password,
                                        static_cast<lv_keyboard_mode_t>(kb_mode), divisor);
            row_ = lv_obj_get_parent(obj_);
        }

        std::string get_text() const
        {
            return obj_ ? std::string(lv_textarea_get_text(obj_)) : "";
        }
        void set_text(Str texto)
        {
            if (obj_)
                lv_textarea_set_text(obj_, texto);
        }
        void set_max_length(uint32_t max_len)
        {
            if (obj_)
                lv_textarea_set_max_length(obj_, max_len);
        }

        SignalProxy<std::string> signal_changed()
        {
            lv_obj_t *ta = obj_;
            return SignalProxy<std::string>([ta](std::function<void(std::string)> cb)
                                            { detail::connect_raw(ta, LV_EVENT_VALUE_CHANGED, [ta, fn = std::move(cb)](lv_event_t *)
                                                                  {
                if (fn) fn(lv_textarea_get_text(ta)); }); });
        }

        SignalProxy<std::string> signal_activate()
        {
            lv_obj_t *ta = obj_;
            return SignalProxy<std::string>([ta](std::function<void(std::string)> cb)
                                            { detail::connect_raw(ta, LV_EVENT_READY, [ta, fn = std::move(cb)](lv_event_t *)
                                                                  {
                if (fn) fn(lv_textarea_get_text(ta)); }); });
        }

    protected:
        lv_obj_t *layout_raw() const noexcept override { return row_ ? row_ : obj_; }

    private:
        lv_obj_t *row_ = nullptr;
    };

    class ValueRow : public Widget
    {
    public:
        ValueRow() = default;
        ValueRow(BoxedList &list, Str titulo, Str subtitulo = "", Str valor_ini = "",
                 lv_color_t color_val = ADW_COLOR_FG, const lv_font_t *font_val = ADW_FONT_HEADING,
                 bool divisor = true)
        {
            obj_ = adw_value_row_create(list.raw(), titulo, subtitulo, valor_ini,
                                        color_val, font_val, divisor);
            row_ = lv_obj_get_parent(obj_);
        }

        void set_value(Str valor)
        {
            if (obj_)
                lv_label_set_text(obj_, valor);
        }
        void set_value_color(lv_color_t color)
        {
            if (obj_)
                lv_obj_set_style_text_color(obj_, color, 0);
        }

    protected:
        lv_obj_t *layout_raw() const noexcept override { return row_ ? row_ : obj_; }

    private:
        lv_obj_t *row_ = nullptr;
    };

    class NavRow : public Widget
    {
    public:
        NavRow() = default;
        NavRow(BoxedList &list, Str lv_symbol_icon, Str titulo, Str subtitulo = "",
               Str estado_der = "", bool divisor = true)
            : Widget(adw_nav_row_create(list.raw(), lv_symbol_icon, titulo,
                                        subtitulo, estado_der, divisor)) {}

        SignalProxy<> signal_activated() { return signal_clicked(); }
    };

    class ComboRow : public Widget
    {
    public:
        ComboRow() = default;
        ComboRow(BoxedList &list, Str titulo, Str subtitulo, Str opciones_newline,
                 uint32_t seleccionada = 0, bool divisor = true)
        {
            obj_ = adw_combo_row_create(list.raw(), titulo, subtitulo, opciones_newline,
                                        seleccionada, divisor);
            row_ = lv_obj_get_parent(obj_);
        }

        uint32_t get_selected() const
        {
            return obj_ ? lv_dropdown_get_selected(obj_) : 0;
        }
        void set_selected(uint32_t idx)
        {
            if (obj_)
                lv_dropdown_set_selected(obj_, idx);
        }
        std::string get_selected_text() const
        {
            if (!obj_)
                return "";
            char buf[128] = {};
            lv_dropdown_get_selected_str(obj_, buf, sizeof(buf));
            return std::string(buf);
        }

        SignalProxy<uint32_t> signal_changed()
        {
            lv_obj_t *dd = obj_;
            return SignalProxy<uint32_t>([dd](std::function<void(uint32_t)> cb)
                                         { detail::connect_raw(dd, LV_EVENT_VALUE_CHANGED, [dd, fn = std::move(cb)](lv_event_t *)
                                                               {
                if (fn) fn(lv_dropdown_get_selected(dd)); }); });
        }

    protected:
        lv_obj_t *layout_raw() const noexcept override { return row_ ? row_ : obj_; }

    private:
        lv_obj_t *row_ = nullptr;
    };

    class SpinRow : public Widget
    {
    public:
        SpinRow() = default;
        SpinRow(BoxedList &list, Str titulo, Str subtitulo, int32_t min_val, int32_t max_val,
                int32_t inicial, uint32_t digitos = 4, uint32_t decimales = 0,
                int32_t paso = 1, bool divisor = true)
        {
            obj_ = adw_spin_row_create(list.raw(), titulo, subtitulo, min_val, max_val,
                                       inicial, digitos, decimales, paso, divisor);
            lv_obj_t *ctrl = lv_obj_get_parent(obj_);
            row_ = ctrl ? lv_obj_get_parent(ctrl) : obj_;
        }

        int32_t get_value() const
        {
            return obj_ ? lv_spinbox_get_value(obj_) : 0;
        }
        void set_value(int32_t val)
        {
            if (obj_)
                lv_spinbox_set_value(obj_, val);
        }

        SignalProxy<int32_t> signal_value_changed()
        {
            lv_obj_t *sb = obj_;
            return SignalProxy<int32_t>([sb](std::function<void(int32_t)> cb)
                                        { detail::connect_raw(sb, LV_EVENT_VALUE_CHANGED, [sb, fn = std::move(cb)](lv_event_t *)
                                                              {
                if (fn) fn(lv_spinbox_get_value(sb)); }); });
        }

    protected:
        lv_obj_t *layout_raw() const noexcept override { return row_ ? row_ : obj_; }

    private:
        lv_obj_t *row_ = nullptr;
    };

    class ListBox : public BoxedList
    {
    public:
        ListBox() = default;
        explicit ListBox(Widget &parent, Str group_title = "")
            : BoxedList(parent, group_title) {}

        void remove_all() { clear_children(); }

        NavRow add_nav_row(Str icon, Str titulo, Str subtitulo, Str estado_der,
                           std::function<void()> on_activate, bool divisor = true)
        {
            NavRow row(*this, icon, titulo, subtitulo, estado_der, divisor);
            if (on_activate)
                row.signal_activated().connect(std::move(on_activate));
            return row;
        }

        ValueRow add_value_row(Str titulo, Str valor_ini = "- - - -", bool divisor = true,
                               lv_color_t color = ADW_COLOR_FG,
                               const lv_font_t *font = ADW_FONT_BODY)
        {
            return ValueRow(*this, titulo, "", valor_ini, color, font, divisor);
        }
    };

    struct ColumnSpec
    {
        const char *title;
        uint8_t weight = 1;
        lv_text_align_t align = LV_TEXT_ALIGN_RIGHT;
    };

    class ColumnView : public Widget
    {
    public:
        ColumnView() = default;
        ColumnView(Widget &parent, const std::vector<ColumnSpec> &cols)
            : cols_(cols)
        {
            obj_ = lv_obj_create(parent.raw());
            lv_obj_remove_style_all(obj_);
            adw_set_scrollable(obj_, false);
            lv_obj_set_size(obj_, lv_pct(100), lv_pct(100));
            lv_obj_set_flex_grow(obj_, 1);
            lv_obj_set_flex_flow(obj_, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_style_radius(obj_, 10, 0);
            lv_obj_set_style_clip_corner(obj_, true, 0);
            lv_obj_set_style_bg_color(obj_, ADW_COLOR_VIEW_BG, 0);
            lv_obj_set_style_bg_opa(obj_, LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(obj_, 1, 0);
            lv_obj_set_style_border_color(obj_, ADW_COLOR_SHADE, 0);
            lv_obj_set_style_border_opa(obj_, 60, 0);

            // Cabecera fija de columnas
            lv_obj_t *hdr = lv_obj_create(obj_);
            lv_obj_remove_style_all(hdr);
            adw_set_scrollable(hdr, false);
            lv_obj_set_size(hdr, lv_pct(100), LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
            lv_obj_set_style_pad_hor(hdr, 12, 0);
            lv_obj_set_style_pad_ver(hdr, 7, 0);
            lv_obj_set_style_pad_column(hdr, 6, 0);
            lv_obj_set_style_bg_color(hdr, ADW_COLOR_FG, 0);
            lv_obj_set_style_bg_opa(hdr, 14, 0);
            lv_obj_set_style_border_width(hdr, 1, 0);
            lv_obj_set_style_border_side(hdr, LV_BORDER_SIDE_BOTTOM, 0);
            lv_obj_set_style_border_color(hdr, ADW_COLOR_SHADE, 0);
            lv_obj_set_style_border_opa(hdr, ADW_OPA_SHADE, 0);

            for (const auto &c : cols_)
            {
                lv_obj_t *lbl = lv_label_create(hdr);
                lv_label_set_text(lbl, c.title);
                lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
                lv_obj_set_height(lbl, LV_SIZE_CONTENT);
                lv_obj_set_flex_grow(lbl, c.weight);
                lv_obj_set_style_text_align(lbl, c.align, 0);
                lv_obj_set_style_text_font(lbl, ADW_FONT_CAPTION, 0);
                lv_obj_set_style_text_color(lbl, ADW_COLOR_FG, 0);
                lv_obj_set_style_text_opa(lbl, ADW_OPA_DIM, 0);
            }

            // Cuerpo con scroll independiente
            body_ = lv_obj_create(obj_);
            lv_obj_remove_style_all(body_);
            lv_obj_set_width(body_, lv_pct(100));
            lv_obj_set_flex_grow(body_, 1);
            lv_obj_set_flex_flow(body_, LV_FLEX_FLOW_COLUMN);
            adw_set_scrollable(body_, true);
        }

        size_t add_row(const std::vector<std::string> &values)
        {
            lv_obj_t *row = lv_obj_create(body_);
            lv_obj_remove_style_all(row);
            adw_set_scrollable(row, false);
            lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_style_pad_hor(row, 12, 0);
            lv_obj_set_style_pad_ver(row, 6, 0);
            lv_obj_set_style_pad_column(row, 6, 0);
            lv_obj_set_style_bg_color(row, ADW_COLOR_ACCENT_BG, 0);
            lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(row, 1, 0);
            lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
            lv_obj_set_style_border_color(row, ADW_COLOR_SHADE, 0);
            lv_obj_set_style_border_opa(row, 48, 0);

            RowData rd{row, {}};
            rd.cells.reserve(cols_.size());

            for (size_t i = 0; i < cols_.size(); ++i)
            {
                lv_obj_t *lbl = lv_label_create(row);
                lv_label_set_text(lbl, i < values.size() ? values[i].c_str() : "");
                lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
                lv_obj_set_height(lbl, LV_SIZE_CONTENT);
                lv_obj_set_flex_grow(lbl, cols_[i].weight);
                lv_obj_set_style_text_align(lbl, cols_[i].align, 0);
                lv_obj_set_style_text_font(lbl, ADW_FONT_MONO, 0);
                lv_obj_set_style_text_color(lbl, ADW_COLOR_FG, 0);
                rd.cells.push_back(lbl);
            }

            rows_.push_back(std::move(rd));
            return rows_.size() - 1;
        }

        void set_cell(size_t row, size_t col, Str text,
                      lv_color_t color = ADW_COLOR_FG, lv_opa_t opa = LV_OPA_COVER)
        {
            if (row >= rows_.size() || col >= rows_[row].cells.size())
                return;
            lv_obj_t *lbl = rows_[row].cells[col];
            lv_label_set_text(lbl, text);
            lv_obj_set_style_text_color(lbl, color, 0);
            lv_obj_set_style_text_opa(lbl, opa, 0);
        }

        void set_row_highlight(size_t row, bool active, lv_color_t color = ADW_COLOR_SUCCESS_TEXT)
        {
            if (row >= rows_.size())
                return;
            lv_obj_set_style_bg_color(rows_[row].row, color, 0);
            lv_obj_set_style_bg_opa(rows_[row].row, active ? 32 : LV_OPA_TRANSP, 0);
        }

        void clear_highlights()
        {
            for (auto &r : rows_)
            {
                lv_obj_set_style_bg_opa(r.row, LV_OPA_TRANSP, 0);
            }
        }

        void clear()
        {
            if (body_)
                lv_obj_clean(body_);
            rows_.clear();
        }

    private:
        struct RowData
        {
            lv_obj_t *row = nullptr;
            std::vector<lv_obj_t *> cells;
        };

        lv_obj_t *body_ = nullptr;
        std::vector<ColumnSpec> cols_;
        std::vector<RowData> rows_;
    };

    class ActionCard : public Widget
    {
    public:
        ActionCard() = default;
        ActionCard(Widget &parent, Str lv_symbol_icon, lv_color_t icon_color,
                   Str titulo, Str subtitulo, int32_t w = 220, int32_t h = 190)
            : Widget(adw_action_card_create(parent.raw(), lv_symbol_icon, icon_color,
                                            titulo, subtitulo, w, h)) {}
    };

    class Numpad : public Widget
    {
    public:
        Numpad() = default;
        Numpad(Widget &parent, Widget &target_textarea, bool allow_decimal = false,
               Str confirm_text = "Confirmar")
            : Widget(adw_numpad_create(parent.raw(), target_textarea.raw(),
                                       allow_decimal, confirm_text)),
              target_ta_(target_textarea.raw()) {}

        SignalProxy<std::string> signal_confirm()
        {
            lv_obj_t *ta = target_ta_;
            return SignalProxy<std::string>([ta](std::function<void(std::string)> cb)
                                            { detail::connect_raw(ta, LV_EVENT_READY, [ta, fn = std::move(cb)](lv_event_t *)
                                                                  {
                if (fn) fn(lv_textarea_get_text(ta)); }); });
        }

    private:
        lv_obj_t *target_ta_ = nullptr;
    };

    class QrCard : public Widget
    {
    public:
        QrCard() = default;
        QrCard(Widget &parent, Str data, int32_t qr_size = 180, Str pairing_code = "")
        {
            obj_ = adw_qr_card_create(parent.raw(), data, qr_size, pairing_code, &qr_);
        }

        void set_data(Str data)
        {
#if LV_USE_QRCODE
            if (qr_ && !data.empty())
            {
                lv_qrcode_update(qr_, data.ptr, std::strlen(data.ptr));
            }
#else
            LV_UNUSED(data);
#endif
        }

    private:
        lv_obj_t *qr_ = nullptr;
    };

    class AlertDialog
    {
    public:
        static void show(Str titulo, Str cuerpo, Str cancel_txt, Str confirm_txt,
                         BtnVariant confirm_variant, std::function<void()> on_confirm)
        {
            detail::g_alert_confirm_cb = std::move(on_confirm);
            adw_alert_dialog_show(
                titulo, cuerpo, cancel_txt, confirm_txt,
                static_cast<adw_btn_variant_t>(confirm_variant),
                detail::g_alert_confirm_cb ? detail::alert_confirm_trampoline : nullptr,
                nullptr);
        }

        static void close()
        {
            detail::g_alert_confirm_cb = nullptr;
            adw_alert_dialog_close();
        }
    };

    class PromptDialog
    {
    public:
        static void show(Str titulo, Str cuerpo, Str placeholder, bool password,
                         Str cancel_txt, Str confirm_txt,
                         std::function<void(std::string)> on_confirm)
        {
            adw_alert_dialog_close();

            lv_obj_t *dim = lv_obj_create(lv_layer_top());
            lv_obj_remove_style_all(dim);
            lv_obj_set_size(dim, lv_pct(100), lv_pct(100));
            lv_obj_set_style_bg_color(dim, ADW_COLOR_SHADE, 0);
            lv_obj_set_style_bg_opa(dim, 165, 0);
            adw_set_clickable(dim, true);

            lv_obj_t *sheet = lv_obj_create(dim);
            lv_obj_remove_style_all(sheet);
            adw_set_scrollable(sheet, false);
            lv_obj_set_size(sheet, 430, LV_SIZE_CONTENT);
            lv_obj_align(sheet, LV_ALIGN_TOP_MID, 0, 28);
            lv_obj_set_flex_flow(sheet, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(sheet, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_style_bg_color(sheet, ADW_COLOR_DIALOG_BG, 0);
            lv_obj_set_style_bg_opa(sheet, LV_OPA_COVER, 0);
            lv_obj_set_style_radius(sheet, 18, 0);
            lv_obj_set_style_pad_all(sheet, 22, 0);
            lv_obj_set_style_pad_row(sheet, 12, 0);

            lv_obj_t *lbl_t = lv_label_create(sheet);
            lv_label_set_text(lbl_t, titulo);
            lv_obj_set_style_text_font(lbl_t, ADW_FONT_TITLE_2, 0);
            lv_obj_set_style_text_color(lbl_t, ADW_COLOR_FG, 0);

            if (!cuerpo.empty())
            {
                lv_obj_t *lbl_b = lv_label_create(sheet);
                lv_label_set_text(lbl_b, cuerpo);
                lv_obj_set_width(lbl_b, lv_pct(100));
                lv_label_set_long_mode(lbl_b, LV_LABEL_LONG_WRAP);
                lv_obj_set_style_text_align(lbl_b, LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_set_style_text_font(lbl_b, ADW_FONT_BODY, 0);
                lv_obj_set_style_text_color(lbl_b, ADW_COLOR_FG, 0);
                lv_obj_set_style_text_opa(lbl_b, 215, 0);
            }

            lv_obj_t *ta = lv_textarea_create(sheet);
            lv_textarea_set_one_line(ta, true);
            lv_textarea_set_password_mode(ta, password);
            lv_textarea_set_placeholder_text(ta, placeholder);
            lv_obj_set_width(ta, lv_pct(100));
            lv_obj_set_style_radius(ta, 9, 0);
            lv_obj_set_style_bg_color(ta, ADW_COLOR_FG, 0);
            lv_obj_set_style_bg_opa(ta, ADW_OPA_CTRL_BG, 0);
            lv_obj_set_style_border_width(ta, 0, 0);
            lv_obj_set_style_text_color(ta, ADW_COLOR_FG, 0);
            lv_obj_set_style_text_font(ta, ADW_FONT_BODY, 0);

            lv_obj_t *kb = lv_keyboard_create(dim);
            lv_obj_set_size(kb, lv_pct(100), lv_pct(42));
            lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
            lv_keyboard_set_textarea(kb, ta);

            lv_obj_t *actions = lv_obj_create(sheet);
            lv_obj_remove_style_all(actions);
            adw_set_scrollable(actions, false);
            lv_obj_set_size(actions, lv_pct(100), LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(actions, LV_FLEX_FLOW_ROW);
            lv_obj_set_style_pad_column(actions, 12, 0);

            lv_obj_t *btn_cancel = adw_button_create(actions, cancel_txt, ADW_BTN_DEFAULT, false);
            lv_obj_set_flex_grow(btn_cancel, 1);
            lv_obj_set_height(btn_cancel, 42);

            lv_obj_t *btn_ok = adw_button_create(actions, confirm_txt, ADW_BTN_SUGGESTED, false);
            lv_obj_set_flex_grow(btn_ok, 1);
            lv_obj_set_height(btn_ok, 42);

            auto submit = std::make_shared<std::function<void()>>(
                [dim, ta, cb = std::move(on_confirm)]() mutable
                {
                    std::string value = lv_textarea_get_text(ta);
                    lv_obj_delete(dim);
                    if (cb)
                        cb(std::move(value));
                });

            detail::connect_raw(btn_cancel, LV_EVENT_CLICKED, [dim](lv_event_t *)
                                { lv_obj_delete(dim); });
            detail::connect_raw(btn_ok, LV_EVENT_CLICKED, [submit](lv_event_t *)
                                { (*submit)(); });
            detail::connect_raw(ta, LV_EVENT_READY, [submit](lv_event_t *)
                                { (*submit)(); });
            detail::connect_raw(ta, LV_EVENT_CANCEL, [dim](lv_event_t *)
                                { lv_obj_delete(dim); });
        }
    };

    class Toast
    {
    public:
        static void show(Str mensaje, uint32_t ms = 2500)
        {
            adw_toast_show(mensaje, ms);
        }
    };

    class LoadingOverlay
    {
    public:
        LoadingOverlay() = default;
        explicit LoadingOverlay(Str texto)
            : overlay_(adw_loading_overlay_create(texto)) {}
        ~LoadingOverlay() { close(); }

        LoadingOverlay(const LoadingOverlay &) = delete;
        LoadingOverlay &operator=(const LoadingOverlay &) = delete;

        LoadingOverlay(LoadingOverlay &&other) noexcept : overlay_(other.overlay_)
        {
            other.overlay_ = nullptr;
        }
        LoadingOverlay &operator=(LoadingOverlay &&other) noexcept
        {
            if (this != &other)
            {
                close();
                overlay_ = other.overlay_;
                other.overlay_ = nullptr;
            }
            return *this;
        }

        void show(Str texto)
        {
            close();
            overlay_ = adw_loading_overlay_create(texto);
        }

        void close()
        {
            if (overlay_)
            {
                lv_obj_delete(overlay_);
                overlay_ = nullptr;
            }
        }

    private:
        lv_obj_t *overlay_ = nullptr;
    };
    class Carousel : public Widget
    {
    public:
        Carousel() = default;
        explicit Carousel(Widget &parent, bool show_dots = true)
            : state_(std::make_shared<State>())
        {
            obj_ = lv_obj_create(parent.raw());
            lv_obj_remove_style_all(obj_);
            adw_set_scrollable(obj_, false);
            lv_obj_set_size(obj_, lv_pct(100), lv_pct(100));
            lv_obj_set_flex_grow(obj_, 1);

            // Riel horizontal a pantalla completa
            lv_obj_t *track = lv_obj_create(obj_);
            lv_obj_remove_style_all(track);
            lv_obj_set_size(track, lv_pct(100), lv_pct(100));
            lv_obj_set_flex_flow(track, LV_FLEX_FLOW_ROW);
            lv_obj_set_scroll_dir(track, LV_DIR_HOR);
            lv_obj_set_scroll_snap_x(track, LV_SCROLL_SNAP_CENTER);
            lv_obj_set_scrollbar_mode(track, LV_SCROLLBAR_MODE_OFF);
            adw_set_clickable(track, true);
#if (LVGL_VERSION_MAJOR > 9) || (LVGL_VERSION_MAJOR == 9 && LVGL_VERSION_MINOR >= 6)
            lv_obj_set_scroll_one(track, true);
#else
            lv_obj_add_flag(track, LV_OBJ_FLAG_SCROLL_ONE);
#endif
            state_->track = track;

            // Píldora flotante translúcida con los puntos indicadores
            if (show_dots)
            {
                lv_obj_t *dots = lv_obj_create(obj_);
                lv_obj_remove_style_all(dots);
                adw_set_scrollable(dots, false);
                adw_set_clickable(dots, false);
                lv_obj_set_size(dots, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                lv_obj_align(dots, LV_ALIGN_BOTTOM_MID, 0, -14);
                lv_obj_set_flex_flow(dots, LV_FLEX_FLOW_ROW);
                lv_obj_set_flex_align(dots, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
                lv_obj_set_style_pad_hor(dots, 12, 0);
                lv_obj_set_style_pad_ver(dots, 7, 0);
                lv_obj_set_style_pad_column(dots, 8, 0);
                lv_obj_set_style_radius(dots, LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_bg_color(dots, ADW_COLOR_SHADE, 0);
                lv_obj_set_style_bg_opa(dots, 130, 0);
                state_->dots_bar = dots;
            }

            std::weak_ptr<State> weak = state_;
            detail::connect_raw(track, LV_EVENT_SCROLL_END, [weak](lv_event_t *)
                                {
            if (auto st = weak.lock()) sync_index_from_scroll(*st); });
        }

        // Agrega un slide vacío (útil para banners con widgets o pantalla de respaldo)
        Box add_slide()
        {
            Box slide(state_->track, Orientation::Vertical, 12);
            lv_obj_set_size(slide.raw(), lv_pct(100), lv_pct(100));
            slide.set_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            adw_set_clickable(slide.raw(), false); // Deja pasar los toques al track
            state_->slides.push_back(slide.raw());

            if (state_->dots_bar)
            {
                lv_obj_t *dot = lv_obj_create(state_->dots_bar);
                lv_obj_remove_style_all(dot);
                adw_set_clickable(dot, false);
                lv_obj_set_size(dot, state_->dots.empty() ? 18 : 8, 8);
                lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_bg_color(dot, state_->dots.empty() ? ADW_COLOR_ACCENT_TEXT : ADW_COLOR_FG, 0);
                lv_obj_set_style_bg_opa(dot, state_->dots.empty() ? LV_OPA_COVER : ADW_OPA_BORDER, 0);
                state_->dots.push_back(dot);
                adw_set_hidden(state_->dots_bar, state_->dots.size() <= 1);
            }
            return slide;
        }

        // Agrega una imagen desde archivo ("A:/ruta/img.jpg" o "S:/sdcard/img.jpg") o lv_image_dsc_t*
        lv_obj_t *add_image_slide(Str lv_src, bool cover = true)
        {
            Box slide = add_slide();
            lv_obj_t *img = lv_image_create(slide.raw());
            lv_image_set_src(img, lv_src);
            lv_obj_set_size(img, lv_pct(100), lv_pct(100));
            lv_image_set_inner_align(img, cover ? LV_IMAGE_ALIGN_COVER : LV_IMAGE_ALIGN_CONTAIN);
            adw_set_clickable(img, false);
            return img;
        }

        void clear_slides()
        {
            if (!state_)
                return;
            if (state_->track)
                lv_obj_clean(state_->track);
            if (state_->dots_bar)
            {
                lv_obj_clean(state_->dots_bar);
                adw_set_hidden(state_->dots_bar, true);
            }
            state_->slides.clear();
            state_->dots.clear();
            state_->current = 0;
        }

        size_t count() const noexcept
        {
            return state_ ? state_->slides.size() : 0;
        }

        void go_to(size_t index, bool anim = true)
        {
            if (!state_ || index >= state_->slides.size())
                return;
            state_->current = index;
            lv_obj_scroll_to_view(state_->slides[index], anim ? LV_ANIM_ON : LV_ANIM_OFF);
            update_dots(*state_);
        }

        void next()
        {
            if (state_ && !state_->slides.empty())
            {
                go_to((state_->current + 1) % state_->slides.size());
            }
        }

        void prev()
        {
            if (state_ && !state_->slides.empty())
            {
                go_to((state_->current + state_->slides.size() - 1) % state_->slides.size());
            }
        }

        void start_autoplay(uint32_t interval_ms)
        {
            if (!state_)
                return;
            if (interval_ms == 0)
            {
                stop_autoplay();
                return;
            }
            std::weak_ptr<State> weak = state_;
            state_->autoplay.start(interval_ms, [weak]()
                                   {
            auto st = weak.lock();
            if (!st) return false;
            if (st->slides.size() > 1) {
                st->current = (st->current + 1) % st->slides.size();
                lv_obj_scroll_to_view(st->slides[st->current], LV_ANIM_ON);
                update_dots(*st);
            }
            return true; });
        }

        void stop_autoplay()
        {
            if (state_)
                state_->autoplay.stop();
        }

        // Se dispara solo con toque limpio (no al deslizar para cambiar de slide)
        SignalProxy<> signal_tap()
        {
            lv_obj_t *target = state_ ? state_->track : nullptr;
            return SignalProxy<>([target](std::function<void()> cb)
                                 { detail::connect_raw(target, LV_EVENT_SHORT_CLICKED, [fn = std::move(cb)](lv_event_t *)
                                                       {
                if (fn) fn(); }); });
        }

    private:
        struct State
        {
            lv_obj_t *track = nullptr;
            lv_obj_t *dots_bar = nullptr;
            std::vector<lv_obj_t *> slides;
            std::vector<lv_obj_t *> dots;
            size_t current = 0;
            Timer autoplay;
        };

        static void sync_index_from_scroll(State &st)
        {
            if (st.slides.empty() || !st.track)
                return;
            int32_t w = lv_obj_get_width(st.track);
            if (w <= 0)
                return;
            int32_t x = lv_obj_get_scroll_x(st.track);
            size_t idx = static_cast<size_t>((x + w / 2) / w);
            if (idx >= st.slides.size())
                idx = st.slides.size() - 1;
            st.current = idx;
            update_dots(st);
        }

        static void update_dots(State &st)
        {
            for (size_t i = 0; i < st.dots.size(); ++i)
            {
                bool active = (i == st.current);
                lv_obj_set_size(st.dots[i], active ? 18 : 8, 8);
                lv_obj_set_style_bg_color(st.dots[i], active ? ADW_COLOR_ACCENT_TEXT : ADW_COLOR_FG, 0);
                lv_obj_set_style_bg_opa(st.dots[i], active ? LV_OPA_COVER : ADW_OPA_BORDER, 0);
            }
        }

        std::shared_ptr<State> state_;
    };

} // namespace adw
#endif // __cplusplus
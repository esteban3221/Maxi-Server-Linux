#include "adw_lvgl.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

// ============================================================================
// COMPONENTE VConfig (reemplazo directo de src/view/config.cpp)
// ============================================================================
class VConfig {
public:
    explicit VConfig(adw::Widget& parent)
        : root_(parent, adw::Orientation::Horizontal, 0)
    {
        root_.set_size_request(lv_pct(100), lv_pct(100));
        root_.set_flex_grow(1);

        build_sidebar();
        build_content_pane();
    }

    // Crea la página en v_stack_ y su botón correspondiente en la barra lateral
    adw::Box add_page(const std::string& id, adw::Str icon, adw::Str title, int32_t pad = 0) {
        adw::Box page = v_stack_.add_page(id, pad);

        SidebarItem item;
        item.id    = id;
        item.title = title.ptr;

        lv_obj_t* btn = lv_button_create(nav_scroll_.raw());
        lv_obj_remove_style_all(btn);
        lv_obj_set_size(btn, lv_pct(100), 44);
        lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(btn, 10, 0);
        lv_obj_set_style_pad_column(btn, 10, 0);
        lv_obj_set_style_radius(btn, 10, 0);
        lv_obj_set_style_bg_color(btn, ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
        lv_obj_set_style_bg_opa(btn, 22, LV_STATE_PRESSED);
        item.btn = btn;

        // Barra vertical de acento (visible solo en el ítem activo)
        lv_obj_t* bar = lv_obj_create(btn);
        lv_obj_remove_style_all(bar);
        adw_set_clickable(bar, false);
        lv_obj_set_size(bar, 3, 20);
        lv_obj_set_style_radius(bar, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(bar, ADW_COLOR_ACCENT_TEXT, 0);
        lv_obj_set_style_bg_opa(bar, LV_OPA_TRANSP, 0);
        item.accent_bar = bar;

        // Ícono de ancho fijo para alinear perfectamente todos los textos
        lv_obj_t* ico = lv_label_create(btn);
        lv_label_set_text(ico, icon);
        lv_obj_set_width(ico, 20);
        lv_obj_set_style_text_align(ico, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(ico, ADW_FONT_ICONS, 0);
        lv_obj_set_style_text_color(ico, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_opa(ico, ADW_OPA_DIM, 0);
        adw_set_clickable(ico, false);
        item.ico = ico;

        // Título de la sección
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, title);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
        lv_obj_set_flex_grow(lbl, 1);
        lv_obj_set_style_text_font(lbl, ADW_FONT_BODY, 0);
        lv_obj_set_style_text_color(lbl, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_opa(lbl, 210, 0);
        adw_set_clickable(lbl, false);
        item.lbl = lbl;

        // Punto opcional de estado (oculto por defecto)
        lv_obj_t* dot = lv_obj_create(btn);
        lv_obj_remove_style_all(dot);
        adw_set_clickable(dot, false);
        lv_obj_set_size(dot, 8, 8);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(dot, ADW_COLOR_SUCCESS_TEXT, 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        adw_set_hidden(dot, true);
        item.status_dot = dot;

        std::string target_id = item.id;
        adw::detail::connect_raw(btn, LV_EVENT_CLICKED, [this, target_id](lv_event_t*) {
            set_visible_child(target_id);
        });

        items_.push_back(std::move(item));

        if (items_.size() == 1) {
            set_visible_child(items_.front().id);
        }
        return page;
    }

    void set_visible_child(const std::string& id) {
        v_stack_.set_visible_child(id);
        active_id_ = id;

        for (auto& it : items_) {
            const bool active = (it.id == id);
            lv_obj_set_style_bg_opa(it.btn, active ? 28 : LV_OPA_TRANSP, 0);
            lv_obj_set_style_bg_opa(it.accent_bar, active ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
            lv_obj_set_style_text_color(it.ico, active ? ADW_COLOR_ACCENT_TEXT : ADW_COLOR_FG, 0);
            lv_obj_set_style_text_opa(it.ico, active ? LV_OPA_COVER : ADW_OPA_DIM, 0);
            lv_obj_set_style_text_font(it.lbl, active ? ADW_FONT_HEADING : ADW_FONT_BODY, 0);
            lv_obj_set_style_text_opa(it.lbl, active ? LV_OPA_COVER : 210, 0);

            if (active) {
                content_header_.set_title(it.title);
            }
        }

        if (on_page_changed_) {
            on_page_changed_(id);
        }
    }

    // Enciende o apaga el punto de estado junto a una sección del menú
    void set_item_status(const std::string& id, lv_color_t color, bool visible = true) {
        for (auto& it : items_) {
            if (it.id == id && it.status_dot) {
                lv_obj_set_style_bg_color(it.status_dot, color, 0);
                adw_set_hidden(it.status_dot, !visible);
                break;
            }
        }
    }

    void set_sidebar_visible(bool visible) {
        sidebar_visible_ = visible;
        adw_set_hidden(sidebar_.raw(), !sidebar_visible_);
    }

    void set_version_text(adw::Str version) {
        lbl_version_.set_text(version);
    }

    adw::HeaderBar& header_bar() noexcept { return content_header_; }

    void set_on_back_clicked(std::function<void()> cb) {
        on_back_ = std::move(cb);
    }

    void set_on_page_changed(std::function<void(const std::string&)> cb) {
        on_page_changed_ = std::move(cb);
    }

private:
    struct SidebarItem {
        std::string id;
        std::string title;
        lv_obj_t* btn        = nullptr;
        lv_obj_t* accent_bar = nullptr;
        lv_obj_t* ico        = nullptr;
        lv_obj_t* lbl        = nullptr;
        lv_obj_t* status_dot = nullptr;
    };

    void build_sidebar() {
        sidebar_ = adw::Box(root_, adw::Orientation::Vertical, 0);
        sidebar_.set_size_request(224, lv_pct(100));
        lv_obj_set_style_bg_color(sidebar_.raw(), ADW_COLOR_HEADERBAR_BG, 0);
        lv_obj_set_style_bg_opa(sidebar_.raw(), LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(sidebar_.raw(), 1, 0);
        lv_obj_set_style_border_side(sidebar_.raw(), LV_BORDER_SIDE_RIGHT, 0);
        lv_obj_set_style_border_color(sidebar_.raw(), ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(sidebar_.raw(), ADW_OPA_SHADE, 0);

        // Cabecera superior de la barra lateral (52 px para empatar con HeaderBar)
        lv_obj_t* top = lv_obj_create(sidebar_.raw());
        lv_obj_remove_style_all(top);
        adw_set_scrollable(top, false);
        lv_obj_set_size(top, lv_pct(100), 52);
        lv_obj_set_flex_flow(top, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(top, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(top, 8, 0);
        lv_obj_set_style_border_width(top, 1, 0);
        lv_obj_set_style_border_side(top, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(top, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(top, ADW_OPA_SHADE, 0);

        // Equivalente a btn_back_config ("Regresar" con go-previous-symbolic)
        lv_obj_t* btn_back = lv_button_create(top);
        lv_obj_remove_style_all(btn_back);
        lv_obj_set_size(btn_back, lv_pct(100), 38);
        lv_obj_set_flex_flow(btn_back, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(btn_back, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(btn_back, 10, 0);
        lv_obj_set_style_pad_column(btn_back, 10, 0);
        lv_obj_set_style_radius(btn_back, 9, 0);
        lv_obj_set_style_bg_color(btn_back, ADW_COLOR_FG, 0);
        lv_obj_set_style_bg_opa(btn_back, LV_OPA_TRANSP, 0);
        lv_obj_set_style_bg_opa(btn_back, ADW_OPA_CTRL_ACT, LV_STATE_PRESSED);

        adw::Widget back_w(btn_back);
        adw::Label ico_back(back_w, LV_SYMBOL_LEFT, ADW_FONT_ICONS, ADW_COLOR_FG);
        adw::Label lbl_back(back_w, "Regresar", ADW_FONT_HEADING, ADW_COLOR_FG);
        adw_set_clickable(ico_back.raw(), false);
        adw_set_clickable(lbl_back.raw(), false);

        adw::detail::connect_raw(btn_back, LV_EVENT_CLICKED, [this](lv_event_t*) {
            if (on_back_) on_back_();
        });

        // Lista scrollable de las secciones
        nav_scroll_ = adw::ScrollView(sidebar_, 8);
        nav_scroll_.set_flex_grow(1);
        lv_obj_set_style_pad_row(nav_scroll_.raw(), 4, 0);

        // Pie discreto del menú
        lv_obj_t* foot = lv_obj_create(sidebar_.raw());
        lv_obj_remove_style_all(foot);
        adw_set_scrollable(foot, false);
        lv_obj_set_size(foot, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_hor(foot, 14, 0);
        lv_obj_set_style_pad_ver(foot, 8, 0);
        lv_obj_set_style_border_width(foot, 1, 0);
        lv_obj_set_style_border_side(foot, LV_BORDER_SIDE_TOP, 0);
        lv_obj_set_style_border_color(foot, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(foot, 48, 0);

        adw::Widget foot_w(foot);
        lbl_version_ = adw::Label(foot_w, "Configuración · MaxiServer", ADW_FONT_CAPTION, ADW_COLOR_FG);
        lbl_version_.set_color(ADW_COLOR_FG, ADW_OPA_DIM);
    }

    void build_content_pane() {
        content_col_ = adw::Box(root_, adw::Orientation::Vertical, 0);
        content_col_.set_flex_grow(1);
        content_col_.set_height(lv_pct(100));

        content_header_ = adw::HeaderBar(content_col_, "General", false);

        // Botón para colapsar/mostrar la barra lateral en vistas anchas
        btn_toggle_sidebar_ = adw::IconButton(content_header_, LV_SYMBOL_BARS, adw::BtnVariant::Flat);
        content_header_.pack_start(btn_toggle_sidebar_);
        btn_toggle_sidebar_.signal_clicked().connect([this]() {
            set_sidebar_visible(!sidebar_visible_);
        });

        v_stack_ = adw::Stack(content_col_);
        v_stack_.set_flex_grow(1);
    }

    adw::Box root_;
    adw::Box sidebar_;
    adw::ScrollView nav_scroll_;
    adw::Label lbl_version_;

    adw::Box content_col_;
    adw::HeaderBar content_header_;
    adw::IconButton btn_toggle_sidebar_;
    adw::Stack v_stack_;

    std::vector<SidebarItem> items_;
    std::string active_id_;
    bool sidebar_visible_ = true;

    std::function<void()> on_back_;
    std::function<void(const std::string&)> on_page_changed_;
};

// ============================================================================
// SIMULADOR INTERACTIVO DE LAS 8 SECCIONES PARA LVGL LIVE PREVIEW
// ============================================================================
class ConfigPreviewHost {
public:
    ConfigPreviewHost()
        : scr_(lv_screen_active())
        , config_(scr_)
    {
        // Mismas 8 páginas ("0" .. "7") que tu src/view/config.cpp
        build_page_0_general(config_.add_page("0", LV_SYMBOL_SETTINGS, "General"));
        build_page_1_info(config_.add_page("1", LV_SYMBOL_LIST, "Información"));
        build_page_2_empresa(config_.add_page("2", LV_SYMBOL_HOME, "Empresa"));
        build_page_3_impresora(config_.add_page("3", LV_SYMBOL_FILE, "Impresora"));
        build_page_4_roles(config_.add_page("4", LV_SYMBOL_EDIT, "Roles"));
        build_page_5_wifi(config_.add_page("5", LV_SYMBOL_WIFI, "Wi-Fi"));
        build_page_6_validadores(config_.add_page("6", LV_SYMBOL_CHARGE, "Validadores"));
        build_page_7_cloud(config_.add_page("7", LV_SYMBOL_UPLOAD, "Maxi Cloud"));

        // Puntos de estado en tiempo real en el menú lateral
        config_.set_item_status("5", ADW_COLOR_SUCCESS_TEXT, true); // Wi-Fi conectado
        config_.set_item_status("6", ADW_COLOR_SUCCESS_TEXT, true); // Validadores OK
        config_.set_item_status("7", ADW_COLOR_WARNING_TEXT, true); // Cloud pendiente

        config_.set_on_back_clicked([]() {
            adw::Toast::show("Regresando a Home (v_main_stack -> \"0\")", 1800);
        });

        config_.set_on_page_changed([](const std::string& id) {
            // Aquí reemplazas los on_show() / signal_map() de cada vista hija
            LV_UNUSED(id);
        });

        scr_.init_keyboard();
    }

private:
    void build_page_0_general(adw::Box page_box) {
        adw::Page p(page_box, 560);
        adw::BoxedList l1(p, "Interfaz");
        adw::SwitchRow(l1, "Mostrar Notificaciones", "Avisos emergentes del sistema", true, true);
        adw::ComboRow(l1, "Temporizador de Carrusel", "", "Ninguno\n5 Seg.\n10 Seg.\n15 Seg.\n30 Seg.\n1 Min.", 2, false);

        adw::BoxedList l2(p, "Seguridad y Energía");
        adw::NavRow(l2, LV_SYMBOL_EDIT, "Establecer PIN de acceso", "", "••••", true);
        adw::NavRow(l2, LV_SYMBOL_REFRESH, "Reiniciar equipo", "", "", true);
        adw::NavRow(l2, LV_SYMBOL_POWER, "Apagar equipo", "", "", false);
    }

    void build_page_1_info(adw::Box page_box) {
        adw::Page p(page_box, 560);
        adw::BoxedList l(p, "Acerca de MaxiServer");
        adw::ValueRow(l, "Versión", "", "2.4.0-lvgl", ADW_COLOR_ACCENT_TEXT, ADW_FONT_MONO, true);
        adw::ValueRow(l, "Motor UI", "", "LVGL 9.6 / Adw-LVGL", ADW_COLOR_FG, ADW_FONT_MONO, true);
        adw::ValueRow(l, "Plataforma", "", "Linux ARM / ESP32-P4", ADW_COLOR_FG, ADW_FONT_MONO, true);
        adw::ValueRow(l, "Estado del Core", "", "Operativo", ADW_COLOR_SUCCESS_TEXT, ADW_FONT_HEADING, false);
    }

    void build_page_2_empresa(adw::Box page_box) {
        adw::Page p(page_box, 560);
        adw::BoxedList l(p, "Datos Fiscales y del Ticket");
        auto r1 = adw::EntryRow(l, "Razón Social", "Nombre de la empresa", false, adw::KeyboardMode::TextUpper, true);
        r1.set_text("MAXICAJERO S.A. DE C.V.");
        auto r2 = adw::EntryRow(l, "RFC", "RFC de la empresa", false, adw::KeyboardMode::TextUpper, true);
        r2.set_text("MXC240101AB1");
        auto r3 = adw::EntryRow(l, "Teléfono", "55-0000-0000", false, adw::KeyboardMode::Number, false);
        r3.set_text("55-1234-5678");
    }

    void build_page_3_impresora(adw::Box page_box) {
        adw::Page p(page_box, 560);
        adw::BoxedList l1(p, "Impresión Térmica");
        adw::SwitchRow(l1, "Activar Impresión", "Emitir ticket al terminar cada venta", true, true);
        adw::NavRow(l1, LV_SYMBOL_FILE, "Impresión de Prueba", "Enviar ticket de diagnóstico", "Probar", false);

        adw::BoxedList l2(p, "Campos en Ticket");
        adw::CheckRow(l2, "Mensaje de Agradecimiento", "", true, true);
        adw::CheckRow(l2, "Fecha y Hora", "", true, true);
        adw::CheckRow(l2, "RFC y Dirección", "", true, false);
    }

    void build_page_4_roles(adw::Box page_box) {
        adw::Page p(page_box, 560);
        adw::BoxedList l(p, "Perfiles de Acceso");
        adw::NavRow(l, LV_SYMBOL_OK, "Administrador", "Acceso total a configuración y cortes", "Activo", true);
        adw::NavRow(l, LV_SYMBOL_EDIT, "Operador / Recarga", "Solo recarga de efectivo (Refill)", "3 permisos", true);
        adw::NavRow(l, LV_SYMBOL_SETTINGS, "Soporte Técnico", "Diagnóstico de validadores y red", "Solo lectura", false);
    }

    void build_page_5_wifi(adw::Box page_box) {
        adw::Page p(page_box, 560);
        adw::BoxedList l(p, "Inalámbrica");
        adw::ValueRow(l, "Red", "", "Sucursal_5G", ADW_COLOR_SUCCESS_TEXT, ADW_FONT_BODY, true);
        adw::ValueRow(l, "Dirección IP", "", "192.168.1.48", ADW_COLOR_FG, ADW_FONT_MONO, true);
        adw::ValueRow(l, "Dirección MAC", "", "DC:A6:32:1A:4F:90", ADW_COLOR_FG, ADW_FONT_MONO, false);
    }

    void build_page_6_validadores(adw::Box page_box) {
        adw::Page p(page_box, 560);
        adw::BoxedList l(p, "Dispositivos Detectados");
        adw::ValueRow(l, "Billetes (SSP 0)", "NV200 Spectral · FW 4.42.0", "Conectado", ADW_COLOR_SUCCESS_TEXT, ADW_FONT_HEADING, true);
        adw::ValueRow(l, "Monedas (SSP 16)", "SMART Hopper · FW 3.18.2", "Conectado", ADW_COLOR_SUCCESS_TEXT, ADW_FONT_HEADING, false);
    }

    void build_page_7_cloud(adw::Box page_box) {
        adw::Page p(page_box, 560);
        adw::BoxedList l(p, "Vinculación Maxi Cloud");
        adw::ValueRow(l, "UUID", "", "9F4A-28C1-B703-E15D", ADW_COLOR_FG, ADW_FONT_MONO, true);
        adw::ValueRow(l, "Estado", "", "Esperando vínculo...", ADW_COLOR_WARNING_TEXT, ADW_FONT_HEADING, true);
        adw::ValueRow(l, "PIN de vinculación", "", "482-915", ADW_COLOR_ACCENT_TEXT, ADW_FONT_MONO_LG, true);
        adw::ComboRow(l, "Canal de actualizaciones", "", "LTS (Estable)\nTest (Pruebas)", 0, false);
    }

    adw::Screen scr_;
    VConfig config_;
};

static std::unique_ptr<ConfigPreviewHost> g_config_app;

static void ui_init(void) {
    adw::UiDispatcher::init();
    g_config_app.reset();
    g_config_app = std::make_unique<ConfigPreviewHost>();
}

#ifdef LVGL_LIVE_PREVIEW
extern "C" void lvgl_live_preview_init(void) {
    ui_init();
}
#endif
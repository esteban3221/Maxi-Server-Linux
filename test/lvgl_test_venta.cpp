#include "adw_lvgl.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>

static std::string fmt_mxn(double val) {
    char buf[40];
    std::snprintf(buf, sizeof(buf), "$%.2f MXN", val < 0.0 ? 0.0 : val);
    return buf;
}

// ============================================================================
// VISTA BASE REUTILIZABLE (equivalente directo a src/view/base.venta_pago.cpp)
// ============================================================================
class BVentaPago {
public:
    explicit BVentaPago(adw::Widget& parent)
        : page_(parent, 560)
    {
        // Compacta márgenes del Page para que quepa holgado incluso en 800x480
        lv_obj_set_style_pad_row(page_.raw(), 14, 0);
        if (lv_obj_t* scroll = lv_obj_get_parent(page_.raw())) {
            lv_obj_set_style_pad_ver(scroll, 14, 0);
            lv_obj_set_style_pad_hor(scroll, 24, 0);
        }

        build_hero();
        build_summary_list();
        build_progress_bar();
        build_footer();
    }

    virtual ~BVentaPago() = default;

    // API cómoda para controladores (Efectivo / Tarjeta)
    void set_titulo(adw::Str texto) { v_lbl_titulo_.set_text(texto); }

    void set_icono(adw::Str symbol, lv_color_t color) {
        lv_obj_set_style_bg_color(badge_icon_, color, 0);
        v_img_main_.set_text(symbol);
        v_img_main_.set_color(color);
    }

    void set_montos(double total, double recibido, double faltante, double cambio) {
        v_lbl_monto_total_.set_value(fmt_mxn(total));
        v_lbl_recibido_.set_value(fmt_mxn(recibido));
        v_lbl_faltante_.set_value(fmt_mxn(faltante));
        v_lbl_cambio_.set_value(fmt_mxn(cambio));

        // Resalta el cambio solo cuando hay monto por devolver
        if (cambio > 0.001) {
            v_lbl_cambio_.set_value_color(ADW_COLOR_WARNING_TEXT);
            lv_obj_set_style_text_opa(v_lbl_cambio_.raw(), LV_OPA_COVER, 0);
        } else {
            v_lbl_cambio_.set_value_color(ADW_COLOR_FG);
            lv_obj_set_style_text_opa(v_lbl_cambio_.raw(), ADW_OPA_DIM, 0);
        }

        // Actualiza barra de progreso (0..1000 para transición suave)
        int32_t pct = (total > 0.0)
            ? static_cast<int32_t>(std::clamp((recibido / total) * 1000.0, 0.0, 1000.0))
            : 1000;
        lv_bar_set_value(bar_progreso_, pct, LV_ANIM_ON);
        lv_obj_set_style_bg_color(
            bar_progreso_,
            (faltante <= 0.001 && total > 0.0) ? ADW_COLOR_SUCCESS_TEXT : ADW_COLOR_ACCENT_BG,
            LV_PART_INDICATOR);
    }

    // Equivalente a v_BXRW4->set_visible(show)
    void set_mostrar_cambio(bool show) {
        v_lbl_cambio_.set_visible(show);
        // Oculta la línea divisoria inferior de "Faltante" si es la última fila visible
        if (lv_obj_t* row_falt = lv_obj_get_parent(v_lbl_faltante_.raw())) {
            lv_obj_set_style_border_width(row_falt, show ? 1 : 0, 0);
        }
    }

    void set_tiempo_restante(int segundos, bool visible = true) {
        adw_set_hidden(badge_timer_, !visible);
        if (!visible) return;

        char buf[16];
        std::snprintf(buf, sizeof(buf), "%ds", std::max(0, segundos));
        lbl_timer_.set_text(buf);

        lv_color_t col = (segundos <= 10) ? ADW_COLOR_WARNING_TEXT : ADW_COLOR_ACCENT_TEXT;
        lv_obj_set_style_bg_color(badge_timer_, col, 0);
        lbl_timer_ico_.set_color(col);
        lbl_timer_.set_color(col);
    }

    void mostrar_mensaje_fin(adw::Str msg, lv_color_t color = ADW_COLOR_SUCCESS_TEXT) {
        v_lbl_mensaje_fin_.set_text(msg);
        v_lbl_mensaje_fin_.set_color(color);
        v_lbl_mensaje_fin_.show();
        box_action_timeout_.hide();
        set_tiempo_restante(0, false);
    }

    void ocultar_mensaje_fin() {
        v_lbl_mensaje_fin_.hide();
        box_action_timeout_.show();
    }

protected:
    // Hooks virtuales idénticos a tu BVentaPago de GTK4
    virtual void on_btn_cancel_click() {}
    virtual void on_btn_retry_click() {}

    adw::Page page_;

    // Encabezado
    lv_obj_t* badge_icon_  = nullptr;
    adw::Label v_img_main_;
    adw::Label v_lbl_titulo_;
    lv_obj_t* badge_timer_ = nullptr;
    adw::Label lbl_timer_ico_;
    adw::Label lbl_timer_;

    // Lista de montos (BXRW1 .. BXRW4)
    adw::BoxedList list_venta_;
    adw::ValueRow v_lbl_monto_total_;
    adw::ValueRow v_lbl_recibido_;
    adw::ValueRow v_lbl_faltante_;
    adw::ValueRow v_lbl_cambio_; // Hace el rol de v_BXRW4 + v_lbl_cambio

    // Progreso, mensaje final y acciones de timeout
    lv_obj_t* bar_progreso_ = nullptr;
    adw::Label v_lbl_mensaje_fin_;
    adw::Box box_action_timeout_;
    adw::Button v_btn_timeout_cancel_;
    adw::Button v_btn_timeout_retry_;

private:
    void build_hero() {
        adw::Box hero(page_, adw::Orientation::Horizontal, 14);
        hero.set_align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        // Insignia circular con el ícono principal
        badge_icon_ = lv_obj_create(hero.raw());
        lv_obj_remove_style_all(badge_icon_);
        adw_set_scrollable(badge_icon_, false);
        lv_obj_set_size(badge_icon_, 48, 48);
        lv_obj_set_style_radius(badge_icon_, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(badge_icon_, ADW_COLOR_ACCENT_TEXT, 0);
        lv_obj_set_style_bg_opa(badge_icon_, 34, 0);

        adw::Widget badge_w(badge_icon_);
        v_img_main_ = adw::Label(badge_w, LV_SYMBOL_CHARGE, ADW_FONT_ICONS, ADW_COLOR_ACCENT_TEXT);
        lv_obj_center(v_img_main_.raw());

        // Título principal
        v_lbl_titulo_ = adw::Label(hero, "Resumen de Pago", ADW_FONT_TITLE_1, ADW_COLOR_FG);
        v_lbl_titulo_.set_flex_grow(1);

        // Píldora de cuenta regresiva (ícono en ADW_FONT_ICONS + texto en ADW_FONT_MONO)
        badge_timer_ = lv_obj_create(hero.raw());
        lv_obj_remove_style_all(badge_timer_);
        adw_set_scrollable(badge_timer_, false);
        lv_obj_set_size(badge_timer_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(badge_timer_, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(badge_timer_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(badge_timer_, 12, 0);
        lv_obj_set_style_pad_ver(badge_timer_, 6, 0);
        lv_obj_set_style_pad_column(badge_timer_, 6, 0);
        lv_obj_set_style_radius(badge_timer_, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(badge_timer_, ADW_COLOR_ACCENT_TEXT, 0);
        lv_obj_set_style_bg_opa(badge_timer_, 32, 0);

        adw::Widget timer_w(badge_timer_);
        lbl_timer_ico_ = adw::Label(timer_w, LV_SYMBOL_LOOP, ADW_FONT_ICONS, ADW_COLOR_ACCENT_TEXT);
        lbl_timer_     = adw::Label(timer_w, "30s", ADW_FONT_MONO, ADW_COLOR_ACCENT_TEXT);
    }

    void build_summary_list() {
        list_venta_ = adw::BoxedList(page_);

        // BXRW1: Total a Pagar (heading + accent grande)
        v_lbl_monto_total_ = adw::ValueRow(
            list_venta_, "Total a Pagar", "", "$0.00 MXN",
            ADW_COLOR_ACCENT_TEXT, ADW_FONT_MONO_LG, true);
        // Resalta el título de la primera fila en negrita (heading)
        if (lv_obj_t* row1 = lv_obj_get_parent(v_lbl_monto_total_.raw())) {
            lv_obj_set_style_min_height(row1, 58, 0);
            if (lv_obj_t* title_box = lv_obj_get_child(row1, 0)) {
                if (lv_obj_t* lbl = lv_obj_get_child(title_box, 0)) {
                    lv_obj_set_style_text_font(lbl, ADW_FONT_HEADING, 0);
                }
            }
        }

        // BXRW2: Recibido (success)
        v_lbl_recibido_ = adw::ValueRow(
            list_venta_, "Recibido", "", "$0.00 MXN",
            ADW_COLOR_SUCCESS_TEXT, ADW_FONT_MONO_LG, true);

        // BXRW3: Faltante (error)
        v_lbl_faltante_ = adw::ValueRow(
            list_venta_, "Faltante", "", "$0.00 MXN",
            ADW_COLOR_ERROR_TEXT, ADW_FONT_MONO_LG, true);

        // BXRW4: Cambio (dim-label por defecto)
        v_lbl_cambio_ = adw::ValueRow(
            list_venta_, "Cambio", "", "$0.00 MXN",
            ADW_COLOR_FG, ADW_FONT_MONO_LG, false);
        lv_obj_set_style_text_opa(v_lbl_cambio_.raw(), ADW_OPA_DIM, 0);
    }

    void build_progress_bar() {
        bar_progreso_ = lv_bar_create(page_.raw());
        lv_bar_set_range(bar_progreso_, 0, 1000);
        lv_bar_set_value(bar_progreso_, 0, LV_ANIM_OFF);
        lv_obj_set_size(bar_progreso_, lv_pct(100), 8);
        lv_obj_set_style_radius(bar_progreso_, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_set_style_radius(bar_progreso_, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(bar_progreso_, ADW_COLOR_FG, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(bar_progreso_, ADW_OPA_CTRL_BG, LV_PART_MAIN);
        lv_obj_set_style_bg_color(bar_progreso_, ADW_COLOR_ACCENT_BG, LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(bar_progreso_, LV_OPA_COVER, LV_PART_INDICATOR);
    }

    void build_footer() {
        // Mensaje final (oculto inicialmente, visible al terminar/cancelar)
        v_lbl_mensaje_fin_ = adw::Label(page_, "", ADW_FONT_TITLE_2, ADW_COLOR_SUCCESS_TEXT);
        v_lbl_mensaje_fin_.set_hexpand(true);
        v_lbl_mensaje_fin_.set_wrap(true);
        lv_obj_set_style_text_align(v_lbl_mensaje_fin_.raw(), LV_TEXT_ALIGN_CENTER, 0);
        v_lbl_mensaje_fin_.hide();

        // Botones pill de timeout (Cancelar Pago / Añadir Tiempo)
        box_action_timeout_ = adw::Box(page_, adw::Orientation::Horizontal, 14);
        box_action_timeout_.set_margin_top(6);
        box_action_timeout_.set_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        v_btn_timeout_cancel_ = adw::Button(
            box_action_timeout_, "Cancelar Pago", adw::BtnVariant::Destructive, true);
        v_btn_timeout_cancel_.set_height(44);
        v_btn_timeout_cancel_.signal_clicked().connect([this]() { on_btn_cancel_click(); });

        v_btn_timeout_retry_ = adw::Button(
            box_action_timeout_, "Añadir Tiempo", adw::BtnVariant::Suggested, true);
        v_btn_timeout_retry_.set_height(44);
        v_btn_timeout_retry_.signal_clicked().connect([this]() { on_btn_retry_click(); });
    }
};

// ============================================================================
// SIMULADOR INTERACTIVO PARA LVGL LIVE PREVIEW
// ============================================================================
class VentaPagoControllerDemo : public BVentaPago {
public:
    explicit VentaPagoControllerDemo(adw::Widget& parent)
        : BVentaPago(parent)
    {
        reiniciar(150.0, false);
    }

    void reiniciar(double nuevo_total, bool tarjeta) {
        modo_tarjeta_ = tarjeta;
        total_        = nuevo_total;
        recibido_     = 0.0;
        segundos_     = 30;
        finalizado_   = false;

        ocultar_mensaje_fin();
        set_mostrar_cambio(!modo_tarjeta_);
        set_titulo(modo_tarjeta_ ? "Pago con Tarjeta" : "Pago en Efectivo");
        set_icono(modo_tarjeta_ ? LV_SYMBOL_WIFI : LV_SYMBOL_CHARGE, ADW_COLOR_ACCENT_TEXT);
        actualizar_ui();
        iniciar_countdown();
    }

    void ingresar(double monto) {
        if (finalizado_) return;
        recibido_ += monto;
        if (modo_tarjeta_ && recibido_ > total_) recibido_ = total_;
        actualizar_ui();

        if (recibido_ >= total_) {
            finalizado_ = true;
            countdown_.stop();
            set_icono(LV_SYMBOL_OK, ADW_COLOR_SUCCESS_TEXT);
            double cambio = std::max(0.0, recibido_ - total_);
            mostrar_mensaje_fin(
                cambio > 0.001
                    ? "¡Pago completado! Retire su cambio y ticket."
                    : "¡Pago completado! Gracias por su compra.",
                ADW_COLOR_SUCCESS_TEXT);
            adw::Toast::show("Venta registrada correctamente", 2200);
        }
    }

    bool es_tarjeta() const { return modo_tarjeta_; }

protected:
    void on_btn_cancel_click() override {
        if (finalizado_) return;
        adw::AlertDialog::show(
            "¿Cancelar pago?",
            recibido_ > 0.0
                ? "Se devolverá el efectivo ingresado hasta el momento."
                : "Se cancelará la operación actual.",
            "Seguir pagando",
            "Cancelar pago",
            adw::BtnVariant::Destructive,
            [this]() {
                finalizado_ = true;
                countdown_.stop();
                set_icono(LV_SYMBOL_CLOSE, ADW_COLOR_ERROR_TEXT);
                mostrar_mensaje_fin("Pago cancelado por el usuario.", ADW_COLOR_ERROR_TEXT);
                adw::Toast::show("Operación cancelada", 2000);
            });
    }

    void on_btn_retry_click() override {
        if (finalizado_) return;
        segundos_ = 30;
        set_tiempo_restante(segundos_, true);
        iniciar_countdown();
        adw::Toast::show("Tiempo extendido +30s", 1600);
    }

private:
    void actualizar_ui() {
        double faltante = std::max(0.0, total_ - recibido_);
        double cambio   = std::max(0.0, recibido_ - total_);
        set_montos(total_, recibido_, faltante, cambio);
        set_tiempo_restante(segundos_, !finalizado_);
    }

    void iniciar_countdown() {
        countdown_.start(1000, [this]() {
            if (finalizado_) return false;
            if (segundos_ > 0) --segundos_;
            set_tiempo_restante(segundos_, true);
            if (segundos_ == 0) {
                finalizado_ = true;
                set_icono(LV_SYMBOL_WARNING, ADW_COLOR_WARNING_TEXT);
                mostrar_mensaje_fin("Tiempo agotado. Intente nuevamente.", ADW_COLOR_WARNING_TEXT);
                return false;
            }
            return true;
        });
    }

    bool modo_tarjeta_ = false;
    bool finalizado_   = false;
    double total_      = 150.0;
    double recibido_   = 0.0;
    int segundos_      = 30;
    adw::Timer countdown_;
};

struct PreviewHost {
    adw::Screen scr;
    VentaPagoControllerDemo vista;
    adw::Box sim_bar;
    adw::Button btn_20;
    adw::Button btn_50;
    adw::Button btn_100;
    adw::Button btn_modo;
    adw::Button btn_reset;

    PreviewHost()
        : scr(lv_screen_active())
        , vista(scr)
        , sim_bar(scr, adw::Orientation::Horizontal, 8)
        , btn_20(sim_bar, "+$20", adw::BtnVariant::Default)
        , btn_50(sim_bar, "+$50", adw::BtnVariant::Default)
        , btn_100(sim_bar, "+$100", adw::BtnVariant::Suggested)
        , btn_modo(sim_bar, "Modo Tarjeta", adw::BtnVariant::Warning)
        , btn_reset(sim_bar, "Reiniciar", adw::BtnVariant::Flat)
    {
        // Barra inferior solo para simular eventos en el Live Preview
        sim_bar.set_align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_bg_color(sim_bar.raw(), ADW_COLOR_HEADERBAR_BG, 0);
        lv_obj_set_style_bg_opa(sim_bar.raw(), LV_OPA_COVER, 0);
        lv_obj_set_style_pad_ver(sim_bar.raw(), 8, 0);
        lv_obj_set_style_pad_hor(sim_bar.raw(), 12, 0);
        lv_obj_set_style_border_width(sim_bar.raw(), 1, 0);
        lv_obj_set_style_border_side(sim_bar.raw(), LV_BORDER_SIDE_TOP, 0);
        lv_obj_set_style_border_color(sim_bar.raw(), ADW_COLOR_SHADE, 0);
        lv_obj_set_style_border_opa(sim_bar.raw(), ADW_OPA_SHADE, 0);

        btn_20.signal_clicked().connect([this]() { vista.ingresar(20.0); });
        btn_50.signal_clicked().connect([this]() { vista.ingresar(50.0); });
        btn_100.signal_clicked().connect([this]() { vista.ingresar(100.0); });
        btn_modo.signal_clicked().connect([this]() {
            bool a_tarjeta = !vista.es_tarjeta();
            vista.reiniciar(150.0, a_tarjeta);
            btn_modo.set_label(a_tarjeta ? "Modo Efectivo" : "Modo Tarjeta");
        });
        btn_reset.signal_clicked().connect([this]() {
            vista.reiniciar(150.0, vista.es_tarjeta());
        });
    }
};

static std::unique_ptr<PreviewHost> g_vp_app;

static void ui_init(void) {
    adw::UiDispatcher::init();
    g_vp_app.reset();
    g_vp_app = std::make_unique<PreviewHost>();
}

#ifdef LVGL_LIVE_PREVIEW
extern "C" void lvgl_live_preview_init(void) {
    ui_init();
}
#endif
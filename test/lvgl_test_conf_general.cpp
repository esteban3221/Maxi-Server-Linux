#include "adw_lvgl.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

// ============================================================================
// HELPER LOCAL: Fila de ruta activable (actualiza subtítulo sin recrear widget)
// ============================================================================
class PathRow : public adw::Widget {
public:
    PathRow() = default;
    PathRow(adw::BoxedList& list, adw::Str icon, adw::Str titulo,
            adw::Str ruta_inicial, adw::Str texto_accion = "Cambiar", bool divisor = true)
    {
        adw::Str sub = ruta_inicial.empty() ? "Sin seleccionar" : ruta_inicial;
        obj_ = adw_row_base_create(list.raw(), titulo, sub, divisor, true);

        // Obtiene el label del subtítulo creado dentro de title_box (hijo 0 -> hijo 1)
        if (lv_obj_t* title_box = lv_obj_get_child(obj_, 0)) {
            lbl_path_ = lv_obj_get_child(title_box, 1);
            if (lbl_path_) {
                lv_obj_set_width(lbl_path_, lv_pct(100));
                lv_label_set_long_mode(lbl_path_, LV_LABEL_LONG_DOT);
                lv_obj_set_style_text_font(lbl_path_, ADW_FONT_MONO, 0);
            }
        }

        // Ícono a la izquierda
        if (!icon.empty()) {
            lv_obj_t* ico = lv_label_create(obj_);
            lv_label_set_text(ico, icon);
            lv_obj_set_style_text_font(ico, ADW_FONT_ICONS, 0);
            lv_obj_set_style_text_color(ico, ADW_COLOR_FG, 0);
            lv_obj_set_style_text_opa(ico, 200, 0);
            lv_obj_move_to_index(ico, 0);
        }

        // Etiqueta tipo píldora a la derecha ("Cambiar" / "Seleccionar")
        if (!texto_accion.empty()) {
            lv_obj_t* pill = lv_obj_create(obj_);
            lv_obj_remove_style_all(pill);
            adw_set_scrollable(pill, false);
            adw_set_clickable(pill, false);
            lv_obj_set_size(pill, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_radius(pill, 8, 0);
            lv_obj_set_style_bg_color(pill, ADW_COLOR_FG, 0);
            lv_obj_set_style_bg_opa(pill, ADW_OPA_CTRL_BG, 0);
            lv_obj_set_style_pad_hor(pill, 10, 0);
            lv_obj_set_style_pad_ver(pill, 5, 0);

            lv_obj_t* lbl_act = lv_label_create(pill);
            lv_label_set_text(lbl_act, texto_accion);
            lv_obj_set_style_text_font(lbl_act, ADW_FONT_CAPTION, 0);
            lv_obj_set_style_text_color(lbl_act, ADW_COLOR_FG, 0);
        }

        lv_obj_t* arrow = lv_label_create(obj_);
        lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
        lv_obj_set_style_text_font(arrow, ADW_FONT_ICONS, 0);
        lv_obj_set_style_text_color(arrow, ADW_COLOR_FG, 0);
        lv_obj_set_style_text_opa(arrow, ADW_OPA_DIM, 0);
    }

    void set_path(adw::Str ruta) {
        path_ = ruta.ptr;
        if (lbl_path_) {
            lv_label_set_text(lbl_path_, path_.empty() ? "Sin seleccionar" : path_.c_str());
        }
    }

    const std::string& get_path() const noexcept { return path_; }

    adw::SignalProxy<> signal_activated() { return signal_clicked(); }

private:
    lv_obj_t* lbl_path_ = nullptr;
    std::string path_;
};

// ============================================================================
// SELECTOR MODAL EMBEBIDO (reemplazo de Gtk::FileDialog en DRM / ESP32-P4)
// ============================================================================
class PickerDialog {
public:
    static void show(adw::Str titulo, adw::Str icon,
                     const std::vector<std::string>& items,
                     std::function<void(std::string)> on_select)
    {
        adw_alert_dialog_close();

        lv_obj_t* dim = lv_obj_create(lv_layer_top());
        lv_obj_remove_style_all(dim);
        lv_obj_set_size(dim, lv_pct(100), lv_pct(100));
        lv_obj_set_style_bg_color(dim, ADW_COLOR_SHADE, 0);
        lv_obj_set_style_bg_opa(dim, 165, 0);
        adw_set_clickable(dim, true);

        lv_obj_t* sheet = lv_obj_create(dim);
        lv_obj_remove_style_all(sheet);
        adw_set_scrollable(sheet, false);
        lv_obj_set_size(sheet, 460, 340);
        lv_obj_center(sheet);
        lv_obj_set_flex_flow(sheet, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_bg_color(sheet, ADW_COLOR_DIALOG_BG, 0);
        lv_obj_set_style_bg_opa(sheet, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(sheet, 18, 0);
        lv_obj_set_style_pad_all(sheet, 18, 0);
        lv_obj_set_style_pad_row(sheet, 12, 0);
        lv_obj_set_style_border_width(sheet, 1, 0);
        lv_obj_set_style_border_color(sheet, ADW_COLOR_FG, 0);
        lv_obj_set_style_border_opa(sheet, 20, 0);

        adw::Widget sheet_w(sheet);
        adw::Box hdr(sheet_w, adw::Orientation::Horizontal, 8);
        hdr.set_align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        adw::Label lbl_t(hdr, titulo, ADW_FONT_TITLE_3, ADW_COLOR_FG);
        lbl_t.set_flex_grow(1);

        adw::IconButton btn_close(hdr, LV_SYMBOL_CLOSE, adw::BtnVariant::Flat);
        btn_close.signal_clicked().connect([dim]() { lv_obj_delete(dim); });

        adw::ScrollView scroll(sheet_w, 0);
        adw::ListBox list(scroll);

        auto cb_ptr = std::make_shared<std::function<void(std::string)>>(std::move(on_select));
        for (size_t i = 0; i < items.size(); ++i) {
            bool divisor = (i + 1 < items.size());
            std::string val = items[i];
            list.add_nav_row(
                icon, val, "", "",
                [dim, cb_ptr, val]() {
                    lv_obj_delete(dim);
                    if (*cb_ptr) (*cb_ptr)(val);
                },
                divisor);
        }
    }
};

// ============================================================================
// VISTA CONFIGURACIÓN GENERAL (equivalente a src/view/config/general.cpp)
// ============================================================================
class VGeneralPreview {
public:
    VGeneralPreview()
        : scr_(lv_screen_active())
        , header_(scr_, "Configuración General", true)
        , page_(scr_, 640)
    {
        build_appearance_group();
        build_carousel_group();
        build_system_group();

        // Inicializar teclado al final para que quede en el fondo del layout de scr_
        scr_.init_keyboard();

        connect_signals();
    }

private:
    static void tint_nav_row(adw::NavRow& row, lv_color_t color, bool tint_bg = false) {
        lv_obj_t* raw = row.raw();
        if (!raw) return;

        // Hijo 0: ícono izquierdo
        if (lv_obj_t* ico = lv_obj_get_child(raw, 0)) {
            lv_obj_set_style_text_color(ico, color, 0);
            lv_obj_set_style_text_opa(ico, LV_OPA_COVER, 0);
        }
        // Hijo 1: title_box -> hijo 0: título
        if (lv_obj_t* title_box = lv_obj_get_child(raw, 1)) {
            if (lv_obj_t* lbl = lv_obj_get_child(title_box, 0)) {
                lv_obj_set_style_text_color(lbl, color, 0);
            }
        }
        if (tint_bg) {
            lv_obj_set_style_bg_color(raw, color, 0);
            lv_obj_set_style_bg_opa(raw, 24, 0);
        }
    }

    void build_appearance_group() {
        list_interfaz_ = adw::BoxedList(page_, "Interfaz y Bienvenida");

        // Reemplaza check_config_notifi
        v_switch_notifi_ = adw::SwitchRow(
            list_interfaz_,
            "Mostrar Notificaciones",
            "Avisos emergentes de cobro, red e impresora",
            true,
            true);

        // Reemplaza btn_select_icon + lbl_path_icon
        v_row_logo_ = PathRow(
            list_interfaz_,
            LV_SYMBOL_IMAGE,
            "Logo de Inicio",
            "/assets/branding/logo_maxicajero.png",
            "Imagen",
            true);

        // Reemplaza ety_mensaje_inicio
        v_ety_mensaje_inicio_ = adw::EntryRow(
            list_interfaz_,
            "Mensaje de Inicio",
            "Ej. ¡Bienvenido!",
            false,
            adw::KeyboardMode::TextLower,
            false);
        v_ety_mensaje_inicio_.set_text("¡Bienvenido! Toque para iniciar");
    }

    void build_carousel_group() {
        list_carrusel_ = adw::BoxedList(page_, "Carrusel y Medios");

        // Reemplaza btn_select_carrousel + lbl_path_carrousel
        v_row_carrousel_ = PathRow(
            list_carrusel_,
            LV_SYMBOL_DIRECTORY,
            "Carpeta de Carrusel",
            "/assets/promos/",
            "Carpeta",
            true);

        // Reemplaza Drop_temporizador con tus 6 opciones exactas
        v_drop_temporizador_ = adw::ComboRow(
            list_carrusel_,
            "Temporizador",
            "Intervalo entre imágenes del protector",
            "Ninguno\n5 Seg.\n10 Seg.\n15 Seg.\n30 Seg.\n1 Min.",
            2,
            true);

        // Reemplaza btn_file_explorer
        v_row_explorer_ = adw::NavRow(
            list_carrusel_,
            LV_SYMBOL_LIST,
            "Gestionar Imágenes",
            "Ver, agregar o quitar anuncios del carrusel",
            "Abrir",
            false);
    }

    void build_system_group() {
        list_sistema_ = adw::BoxedList(page_, "Seguridad y Sistema");

        v_row_pin_ = adw::NavRow(
            list_sistema_,
            LV_SYMBOL_EDIT,
            "Establecer PIN de acceso",
            "Protege el panel de configuración y cortes",
            "••••",
            true);

        v_row_reboot_ = adw::NavRow(
            list_sistema_,
            LV_SYMBOL_REFRESH,
            "Reiniciar",
            "Reinicia el sistema operativo del cajero",
            "",
            true);

        v_row_shutdown_ = adw::NavRow(
            list_sistema_,
            LV_SYMBOL_POWER,
            "Apagar",
            "Apaga el equipo de forma segura",
            "",
            true);

        v_row_factory_ = adw::NavRow(
            list_sistema_,
            LV_SYMBOL_WARNING,
            "Restablecer de Fábrica",
            "Borra configuración local, red y credenciales",
            "",
            false);

        tint_nav_row(v_row_reboot_,   ADW_COLOR_ACCENT_TEXT,  false);
        tint_nav_row(v_row_shutdown_, ADW_COLOR_WARNING_TEXT, false);
        tint_nav_row(v_row_factory_,  ADW_COLOR_ERROR_TEXT,   true);
    }

    void connect_signals() {
        header_.signal_back_clicked().connect([]() {
            adw::Toast::show("Regresando al menú de configuración...", 1800);
        });

        v_switch_notifi_.signal_toggled().connect([](bool en) {
            adw::Toast::show(en ? "Notificaciones activadas" : "Notificaciones desactivadas", 1600);
        });

        v_ety_mensaje_inicio_.signal_activate().connect([](std::string txt) {
            adw::Toast::show("Mensaje guardado: " + txt, 2000);
        });

        v_row_logo_.signal_activated().connect([this]() {
            PickerDialog::show(
                "Seleccionar Logo de Inicio",
                LV_SYMBOL_IMAGE,
                {
                    "/assets/branding/logo_maxicajero.png",
                    "/assets/branding/logo_sucursal.png",
                    "/assets/branding/logo_dark.jpg",
                    "/media/usb/mi_logo.png"
                },
                [this](std::string path) {
                    v_row_logo_.set_path(path);
                    adw::Toast::show("Logo actualizado", 1800);
                });
        });

        v_row_carrousel_.signal_activated().connect([this]() {
            PickerDialog::show(
                "Seleccionar Carpeta de Carrusel",
                LV_SYMBOL_DIRECTORY,
                {
                    "/assets/promos/",
                    "/assets/promos_temporada/",
                    "/media/usb/anuncios/",
                    "/sdcard/carrousel/"
                },
                [this](std::string dir) {
                    v_row_carrousel_.set_path(dir);
                    adw::Toast::show("Carpeta de carrusel actualizada", 1800);
                });
        });

        v_drop_temporizador_.signal_changed().connect([this](uint32_t idx) {
            std::string label = v_drop_temporizador_.get_selected_text();
            adw::Toast::show(idx == 0 ? "Carrusel desactivado" : "Temporizador: " + label, 1800);
        });

        v_row_explorer_.signal_activated().connect([this]() {
            PickerDialog::show(
                "Imágenes en " + v_row_carrousel_.get_path(),
                LV_SYMBOL_IMAGE,
                {
                    "01_bienvenida.jpg  (800x480 · 72 KB)",
                    "02_pago_efectivo.jpg  (800x480 · 84 KB)",
                    "03_recargas_24h.jpg  (800x480 · 65 KB)"
                },
                [](std::string img) {
                    adw::Toast::show("Vista previa: " + img, 2000);
                });
        });

        v_row_pin_.signal_activated().connect([]() {
            adw::PromptDialog::show(
                "Establecer PIN de acceso",
                "Ingresa un nuevo PIN numérico (4 a 6 dígitos).",
                "Nuevo PIN",
                true,
                "Cancelar",
                "Guardar",
                [](std::string pin) {
                    if (pin.size() < 4) {
                        adw::Toast::show("El PIN debe tener al menos 4 dígitos");
                        return;
                    }
                    adw::Toast::show("PIN actualizado correctamente");
                });
        });

        v_row_reboot_.signal_activated().connect([this]() {
            adw::AlertDialog::show(
                "¿Reiniciar el equipo?",
                "Se cerrarán las sesiones activas y el cajero volverá a iniciar en unos segundos.",
                "Cancelar",
                "Reiniciar",
                adw::BtnVariant::Suggested,
                [this]() {
                    loading_.show("Reiniciando sistema...");
                    sim_timer_.start(1200, [this]() {
                        loading_.close();
                        adw::Toast::show("Reinicio simulado completado");
                        return false;
                    });
                });
        });

        v_row_shutdown_.signal_activated().connect([this]() {
            adw::AlertDialog::show(
                "¿Apagar el equipo?",
                "El cajero dejará de operar hasta que vuelva a encenderse físicamente.",
                "Cancelar",
                "Apagar",
                adw::BtnVariant::Warning,
                [this]() {
                    loading_.show("Apagando equipo...");
                    sim_timer_.start(1200, [this]() {
                        loading_.close();
                        adw::Toast::show("Apagado simulado completado");
                        return false;
                    });
                });
        });

        v_row_factory_.signal_activated().connect([this]() {
            adw::AlertDialog::show(
                "¿Restablecer de fábrica?",
                "Esta acción borrará la base de datos de configuración, el PIN y las redes guardadas. No se puede deshacer.",
                "Cancelar",
                "Restablecer",
                adw::BtnVariant::Destructive,
                [this]() {
                    loading_.show("Restableciendo valores de fábrica...");
                    sim_timer_.start(1400, [this]() {
                        loading_.close();
                        adw::Toast::show("Equipo restablecido de fábrica");
                        return false;
                    });
                });
        });
    }

    adw::Screen scr_;
    adw::HeaderBar header_;
    adw::Page page_;

    adw::BoxedList list_interfaz_;
    adw::SwitchRow v_switch_notifi_;
    PathRow v_row_logo_;
    adw::EntryRow v_ety_mensaje_inicio_;

    adw::BoxedList list_carrusel_;
    PathRow v_row_carrousel_;
    adw::ComboRow v_drop_temporizador_;
    adw::NavRow v_row_explorer_;

    adw::BoxedList list_sistema_;
    adw::NavRow v_row_pin_;
    adw::NavRow v_row_reboot_;
    adw::NavRow v_row_shutdown_;
    adw::NavRow v_row_factory_;

    adw::LoadingOverlay loading_;
    adw::Timer sim_timer_;
};

static std::unique_ptr<VGeneralPreview> g_general_app;

static void ui_init(void) {
    adw::UiDispatcher::init();
    g_general_app.reset();
    g_general_app = std::make_unique<VGeneralPreview>();
}

#ifdef LVGL_LIVE_PREVIEW
extern "C" void lvgl_live_preview_init(void) {
    ui_init();
}
#endif
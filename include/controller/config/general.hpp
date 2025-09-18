#pragma once
#include "view/config/general.hpp"
#include "configuracion.hpp"
#include "global.hpp"
#include "carrousel.hpp"


class General : public VGeneral
{
private:
    /* data */
    void carga_estado_inicial();
    void on_list_activate(Gtk::ListBoxRow *row);
    void on_list_checked(Gtk::ListBoxRow *row);
    void on_click_reboot();
    void on_click_shutdown();
    void on_rest_app();
    void init_carrousel();
    void obtener_tiempo_carrousel();

    void on_button_file_clicked();
    void on_button_directory_clicked();
    void on_button_file_explorer_clicked();
    void on_dropdown_directory_time_selected();
    void on_button_image_clicked();

    void on_file_dialog_response(int response_id, Gtk::FileChooserDialog *dialog);
    void on_ety_changed();
    std::vector<std::string> listar_contenido(const std::string& ruta_carpeta);

    /// @brief true para directorio , false para imagen
    bool type;

public:
    General(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~General();
};

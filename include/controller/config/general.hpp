#pragma once
#include "view/config/general.hpp"
#include "configuracion.hpp"
#include "global.hpp"

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

    void on_button_file_clicked();
    void on_file_dialog_response(int response_id, Gtk::FileChooserDialog *dialog);
    void on_ety_changed();

public:
    General(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~General();
};

#pragma once
#include <gtkmm.h>

class VGeneral : public Gtk::Box
{
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    Gtk::Button *v_btn_select_carrousel = nullptr, *v_btn_select_icon = nullptr;
    Gtk::Label *v_lbl_path_icon = nullptr, *v_lbl_path_carrousel = nullptr;
    Gtk::CheckButton *v_check_config_notifi = nullptr;

    Gtk::ListBox *v_list_configurable = nullptr;
    Gtk::ListBox *v_list_config_general = nullptr;

    Gtk::Entry *v_ety_mensaje_inicio = nullptr;
    Gtk::DropDown *v_Drop_temporizador = nullptr;

    std::unique_ptr<Gtk::MessageDialog> v_dialog;

public:
    VGeneral(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    virtual ~VGeneral();
};

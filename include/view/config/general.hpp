#pragma once
#include <gtkmm.h>

class VGeneral : public Gtk::Box
{
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    Gtk::Button *btn_select_carrousel = nullptr, *btn_select_icon = nullptr;
    Gtk::Label *lbl_path_icon = nullptr, *lbl_path_carrousel = nullptr;
    Gtk::CheckButton *check_config_notifi = nullptr;
    Gtk::Box *page_0 = nullptr;

    Gtk::ListBox *list_configurable = nullptr;
    Gtk::ListBox *list_config_general = nullptr;

    Gtk::Entry *ety_mensaje_inicio = nullptr;
    Gtk::DropDown *Drop_temporizador = nullptr;

    std::unique_ptr<Gtk::MessageDialog> dialog;

public:
    VGeneral(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    virtual ~VGeneral();
};

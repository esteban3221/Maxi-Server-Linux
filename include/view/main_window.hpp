#pragma once
#include <gtkmm.h>

class VMainWindow : public Gtk::Window
{

protected:
    
    Gtk::Box *v_box_principal = nullptr;
    Gtk::Label *v_lbl_version = nullptr, *v_lbl_main = nullptr;
    Gtk::Button *v_btn_logo_nip = nullptr, *v_btn_pill = nullptr;
    Gtk::Image *v_img_main_logo = nullptr;

    const char *ui;
public:
    VMainWindow(/* args */);
    ~VMainWindow();
};

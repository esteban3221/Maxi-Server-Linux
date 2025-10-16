#pragma once
#include <gtkmm.h>

class VWifi : public Gtk::Box
{
private:
    Glib::RefPtr<Gtk::Builder> m_builder;

protected:
    Gtk::EditableLabel *v_lbl_red[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};
    Gtk::Button *v_btn_red = nullptr, *v_btn_redes, *v_btn_regresar, *v_btn_refresh = nullptr;
    Gtk::Stack *v_stack_wifi = nullptr;
    Gtk::ListBox /**v_list_info_red = nullptr,*/ *v_list_box_wifi = nullptr;

    class VWifiRow : public Gtk::ListBoxRow
    {
    public:
        Gtk::Label *v_SSID;

        VWifiRow(const std::string &titulo,const std::string &subtitulo);
    private:
        Gtk::Label *v_subtitulo;
        Gtk::Image *v_image_wifi, *v_image_check;
        Gtk::Box *v_box;
    };

public:
    VWifi(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VWifi();
};

namespace View
{
   extern const char *ui_wifi;
}

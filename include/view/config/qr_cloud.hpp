#pragma once
#include <gtkmm.h>
#include <vector>
#include <string>
#include <cmath>

#include <cairomm/cairomm.h>
#include "qrencode.h"

class VQrCloud : public Gtk::Box
{
protected:
    Gtk::Picture *v_picture_qr = nullptr;
    Gtk::Label *v_label_uuid = nullptr, *v_label_pin = nullptr, *v_label_status = nullptr;
    Gtk::DropDown *v_dropdown_canal = nullptr;

    void actualizar_qr(const std::string &url);

public:
    VQrCloud(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~VQrCloud();

private:
    Glib::RefPtr<Gtk::Builder> m_builder;
    void draw_rounded_rect(const Cairo::RefPtr<Cairo::Context> &cr, double x, double y, double width, double height, double radius);
    Glib::RefPtr<Gdk::Texture> crear_qr_estilizado(const std::string &texto, int module_size);
};

namespace View
{
    extern const char *ui_qr_cloud;
}
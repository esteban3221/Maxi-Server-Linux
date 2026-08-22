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
    Gtk::Picture v_picture;
    void actualizar_qr(const std::string &url);

public:
    VQrCloud();
    ~VQrCloud();

private:
    void draw_rounded_rect(const Cairo::RefPtr<Cairo::Context> &cr, double x, double y, double width, double height, double radius);
    Glib::RefPtr<Gdk::Texture> crear_qr_estilizado(const std::string &texto, int module_size);
};
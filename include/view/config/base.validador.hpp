#pragma once
#include <gtkmm.h>
#include <crow.h>
#include <filesystem>

#include "model/configuracion.hpp"

class VValidador : public Gtk::Box
{
protected:
    Gtk::Label *v_lbl_data [11] = {nullptr};
    Gtk::Label *v_lbl_titulo = nullptr;
    Gtk::DropDown *v_drop_puerto = nullptr;

    Gtk::Image *v_img_validador = nullptr;

    void on_show();    
private:
    Glib::RefPtr<Gtk::Builder> m_builder;
    Glib::RefPtr<Gtk::StringList> m_list_puertos;
    std::vector<Glib::ustring> puertos;

    uint16_t id_conf_;

    void llena_puertos();
public:
    VValidador(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> & refBuilder);
    virtual ~VValidador();

    void set_id_conf(uint16_t id);
    void set_data_lbl(const crow::json::rvalue &json);
    void set_data_lbl(const std::string &json);
    void set_img(const std::string &path);
    Glib::ustring get_puerto_seleccionado();
};

namespace View
{
   extern const char *ui_val;
}

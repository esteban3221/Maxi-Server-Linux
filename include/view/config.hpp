#pragma once
#include <gtkmm.h>

#include "controller/config/validador.hpp"
#include "controller/config/general.hpp"
#include "controller/config/impresora.hpp"
#include "controller/config/empresa.hpp"
#include "controller/config/info.hpp"
#include "controller/config/roles.hpp"
#include "controller/config/wifi.hpp"

class VConfig : public Gtk::Box
{
protected:
    Gtk::Button *v_btn_back_config = nullptr;
    Gtk::Box *v_left_box = nullptr;
    Gtk::StackSidebar *v_stack_side = nullptr;
    Gtk::Stack v_stack;
    Gtk::ScrolledWindow v_scroll;
public:
    VConfig(/* args */);
    ~VConfig();
};

#pragma once
#include <gtkmm.h>

#include "controller/config/validador.hpp"

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

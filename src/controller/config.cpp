#include "controller/config.hpp"

Config::Config()
{
    v_btn_back_config->signal_clicked().connect([](){Global::Widget::v_main_stack->set_visible_child("0");});
}

Config::~Config()
{
}
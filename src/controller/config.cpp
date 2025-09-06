#include "controller/config.hpp"
#include "carrousel.hpp"


Config::Config()
{
    v_btn_back_config->signal_clicked().connect([](){Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home);});
}

Config::~Config()
{
}
#include "controller/config/info.hpp"
#include "info.hpp"

Info::Info(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VInfo(cobject, refBuilder)
{
    init_data();
}

Info::~Info()
{
}


void Info::init_data()
{
    const std::string &parent{"cat /sys/devices/virtual/dmi/id/"};
    const std::string &a{parent + "board_vendor"};
    const std::string &b{parent + "product_name"};

    v_list_info_system[0]->set_text(Global::System::exec("cat /etc/hostname"));
    v_list_info_system[1]->set_text((Global::System::exec(a.c_str())).empty() ? 
    Global::System::exec("cat /proc/device-tree/model") : 
    Global::System::exec(a.c_str()) + " " + Global::System::exec(b.c_str()));
    v_list_info_system[2]->set_text(Global::System::exec("lscpu | grep -E 'Nombre del modelo|Model name' | awk -F': ' '{print $2}'"));
    v_list_info_system[3]->set_text(Global::System::exec("grep MemTotal /proc/meminfo | awk '{print $2/1024/1024 \" GB\"}' "));
    v_list_info_system[4]->set_text(Global::System::exec("lsblk -o SIZE -b | head -2 | tail -1 | awk '{print $1/1024/1024/1024 \" GB\"}'"));
}
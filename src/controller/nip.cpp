#include "controller/nip.hpp"
#include "carrousel.hpp"

Nip::Nip(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VBaseNip(cobject, refBuilder) 
{
}

Nip::~Nip()
{
}

void Nip::on_btn_nip_enter()
{
    auto db_conf = std::make_unique<Configuracion>();
    auto data = db_conf->get_conf_data(100,100);

    if(data->get_item(0)->m_valor == v_ety_pin->get_text())
    {
        Global::Widget::v_main_stack->set_visible_child("5");
        v_ety_pin->property_placeholder_text() = "Ingrese NIP";
        v_ety_pin->set_css_classes({"entry","title-1"});
    }
    else
    {
        v_ety_pin->property_placeholder_text() = "Error";
        v_ety_pin->set_css_classes({"error","title-1"});
    }
    
    v_ety_pin->set_text("");
}

void Nip::on_btn_nip_back()
{
    Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home);
}

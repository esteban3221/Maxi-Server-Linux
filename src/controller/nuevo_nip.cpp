#include "controller/nuevo_nip.hpp"
#include "nuevo_nip.hpp"

NuevoNip::NuevoNip(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VBaseNip(cobject, refBuilder) 
{
    this->signal_map().connect(sigc::mem_fun(*this, &NuevoNip::borra_cache));
}

NuevoNip::~NuevoNip()
{
}

void NuevoNip::on_btn_nip_enter()
{
    if (not nip.empty() and nip == v_ety_pin->get_text())
    {
        auto db = std::make_unique<Configuracion>();
        db->update_conf(MConfiguracion::create(100, "Nip Actualizado", nip));
        Global::System::showNotify("Configuracion","Pin actualizado exitosamente.","dialog-information");
        Global::Widget::v_main_stack->set_visible_child("5");
    }
    else if(auto n_nip = v_ety_pin->get_text(); n_nip.size() == 6)
    {
        nip.assign(n_nip);
        v_ety_pin->delete_text(0,-1);
        v_ety_pin->property_placeholder_text() = "Confirme Pin";
    }
    else
    {
        v_ety_pin->delete_text(0,-1);
        Global::System::showNotify("Configuracion","Pin debe de tener 6 caracteres.","dialog-error");
    }
}

void NuevoNip::on_btn_nip_back()
{
    Global::Widget::v_main_stack->set_visible_child("5");
}

void NuevoNip::borra_cache()
{
    nip.clear();
    v_ety_pin->delete_text(0,-1);
    v_ety_pin->property_placeholder_text() = "Digite el nuevo Pin";
}
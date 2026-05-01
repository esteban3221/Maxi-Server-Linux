#include "dialmonto.hpp"

DialMonto::DialMonto(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VBaseNip(cobject, refBuilder)
{
    set_modo_monto(true);
    signal_map().connect(sigc::mem_fun(*this, &DialMonto::on_map_show_map));
    v_spn_monto->signal_activate().connect(sigc::mem_fun(*this, &DialMonto::on_btn_nip_enter));
}

DialMonto::~DialMonto()
{
}

void DialMonto::on_map_show_map()
{
    v_spn_monto->set_text("Ingrese el monto a pagar, Restante: " + std::to_string(monto));
}

void DialMonto::on_btn_nip_enter()
{
    v_spn_monto->update();
    
    try
    {
        signal_monto_entered.emit(v_spn_monto->get_value_as_int());
    }
    catch (const std::exception &e)
    {
        v_ety_pin->property_placeholder_text() = "Error: Ingrese un número válido";
        v_ety_pin->set_css_classes({"error", "title-1"});
    }

    v_ety_pin->set_text("");
}

void DialMonto::on_btn_nip_back()
{
    Global::Widget::v_main_stack->set_visible_child("3");
}
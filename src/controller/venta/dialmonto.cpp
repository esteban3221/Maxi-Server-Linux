#include "dialmonto.hpp"

DialMonto::DialMonto(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VBaseNip(cobject, refBuilder)
{
    prepend(v_lbl_monto);
    prepend(v_lbl_metodo);
    v_lbl_monto.set_css_classes({"dim-label", "title-2"});
    v_lbl_monto.set_lines(2);
    v_lbl_metodo.set_css_classes({"dim-label", "title-1"});
    set_modo_monto(true);
    v_spn_monto->set_increments(1, 10);
    signal_map().connect(sigc::mem_fun(*this, &DialMonto::on_map_show_map));
    v_spn_monto->signal_activate().connect(sigc::mem_fun(*this, &DialMonto::on_btn_nip_enter));
}

DialMonto::~DialMonto()
{
}

void DialMonto::on_map_show_map()
{
    v_lbl_metodo.set_text("Método: " + metodo_nombre);
    v_lbl_monto.set_text("Ingrese el monto a pagar\nRestante: $" + std::to_string(monto));
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
        Global::System::showNotify("Error", "Error: Ingrese un número válido", "dialog-error");
    }
}

void DialMonto::on_btn_nip_back()
{
    Global::Widget::v_main_stack->set_visible_child("3");
}
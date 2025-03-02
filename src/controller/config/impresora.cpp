#include "controller/config/impresora.hpp"
#include "impresora.hpp"

Impresora::Impresora(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VImpresora(cobject, refBuilder)
{
    init_data();

    v_list_config_visualizacion->signal_row_activated().connect(sigc::mem_fun(*this, &Impresora::activa_visualizacion));
    v_list_config->signal_row_activated().connect(sigc::mem_fun(*this, &Impresora::activa_impresion));
    v_list_config_test->signal_row_activated().connect(sigc::mem_fun(*this, &Impresora::activa_test));

    v_switch_impresion->property_active().signal_changed().connect(sigc::mem_fun(*this, &Impresora::estado_checkbutton));
    for (auto &&i : v_check_config)
        i->property_active().signal_changed().connect(sigc::mem_fun(*this, &Impresora::estado_checkbutton));
    
}

Impresora::~Impresora()
{
}

void Impresora::init_data()
{
    auto db = std::make_unique<Configuracion>();
    auto db_impresora = db->get_conf_data(15, 21);

    auto activa_impresion = db_impresora->get_item(0)->m_valor;
    v_switch_impresion->set_active(Global::System::stob(activa_impresion));

    for (size_t i = 1; i < db_impresora->get_n_items(); i++)
        v_check_config[i - 1]->set_active(Global::System::stob(db_impresora->get_item(i)->m_valor));
    
}

void Impresora::activa_impresion(Gtk::ListBoxRow *row)
{
    v_switch_impresion->get_active() ? v_switch_impresion->set_active(false) : v_switch_impresion->set_active();
}

void Impresora::activa_visualizacion(Gtk::ListBoxRow *row)
{
    v_check_config[row->get_index()]->get_active() ? v_check_config[row->get_index()]->set_active(false) : v_check_config[row->get_index()]->set_active();
}

void Impresora::activa_test(Gtk::ListBoxRow *row)
{
    //std::string command = "echo \"" + "Cosas" + "\" | lp";
}

void Impresora::estado_checkbutton()
{
    auto db = std::make_unique<Configuracion>();
    //auto db_impresora = db->get_conf_data(15, 21);

    auto activa_impresion = MConfiguracion::create(15, "Activa Impresion", std::to_string(v_switch_impresion->get_active()));
    db->update_conf(activa_impresion);

    for (size_t i = 1; i < 7; i++)
        db->update_conf(MConfiguracion::create(15 + i, "Visualiza Impresion", std::to_string(v_check_config[i - 1]->get_active())));
    
}

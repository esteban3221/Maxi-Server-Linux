#include "controller/config/impresora.hpp"
#include "impresora.hpp"

Impresora::Impresora(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VImpresora(cobject, refBuilder)
{
    init_data();

    v_list_config_visualizacion->signal_row_activated().connect(sigc::mem_fun(*this, &Impresora::activa_visualizacion));
    v_list_config->signal_row_activated().connect(sigc::mem_fun(*this, &Impresora::activa_impresion));
    v_list_config_test->signal_row_activated().connect(sigc::mem_fun(*this, &Impresora::activa_test));

    v_text_ticket->set_buffer(text_buffer);
    v_text_ticket->signal_map().connect(sigc::mem_fun(*this, &Impresora::actualiza_buffer));

    Global::Widget::Impresora::v_switch_impresion->property_active().signal_changed().connect(sigc::mem_fun(*this, &Impresora::estado_checkbutton));
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
    Global::Widget::Impresora::v_switch_impresion->set_active(Global::System::stob(activa_impresion));

    for (size_t i = 1; i < db_impresora->get_n_items(); i++)
    {
        Global::Widget::Impresora::state_vizualizacion[i] = Global::System::stob(db_impresora->get_item(i)->m_valor);
        v_check_config[i - 1]->set_active(Global::Widget::Impresora::state_vizualizacion[i]);
    }
}

void Impresora::activa_impresion(Gtk::ListBoxRow *row)
{
    Global::Widget::Impresora::v_switch_impresion->get_active() ? Global::Widget::Impresora::v_switch_impresion->set_active(false) : Global::Widget::Impresora::v_switch_impresion->set_active();
}

void Impresora::activa_visualizacion(Gtk::ListBoxRow *row)
{
    Global::Widget::Impresora::state_vizualizacion[row->get_index()] ? v_check_config[row->get_index()]->set_active(false) : v_check_config[row->get_index()]->set_active();
    v_check_config[row->get_index()]->set_active(Global::Widget::Impresora::state_vizualizacion[row->get_index()]);
}

void Impresora::activa_test(Gtk::ListBoxRow *row)
{
    Global::System::showNotify("Impresión", "Se imprimio ticket de prueba.", "dialog-information");
    std::string command = "echo \"" + text_buffer->get_text() + "\" | lp";
    std::system(command.c_str());
}

void Impresora::estado_checkbutton()
{
    auto db = std::make_unique<Configuracion>();

    auto activa_impresion = MConfiguracion::create(15, "Activa Impresion", std::to_string(Global::Widget::Impresora::v_switch_impresion->get_active()));
    db->update_conf(activa_impresion);

    for (size_t i = 1; i < 7; i++)
        db->update_conf(MConfiguracion::create(15 + i, "Visualiza Impresion", std::to_string(v_check_config[i - 1]->get_active())));

    actualiza_buffer();
}

std::string Impresora::test_text_impresion()
{
    std::stringstream ticket_config;
    auto db = std::make_unique<Configuracion>();
    auto db_empresa = db->get_conf_data(10, 14);

    ticket_config << "****** TICKET DE COMPRA ******\n"
                  << "--------------------------------\n\n"
                  << std::left << std::setw(20) << db_empresa->get_item(0)->m_valor << "\n\n";

    if (v_check_config[2]->get_active())
        ticket_config << "Direccion: " << db_empresa->get_item(1)->m_valor << "\n"
                      << "--------------------------------\n";

    if (v_check_config[3]->get_active())
        ticket_config << "RFC: " << db_empresa->get_item(2)->m_valor << "\n"
                      << "--------------------------------\n";

    if (v_check_config[1]->get_active())
        ticket_config << "Fecha: "
                      << Glib::DateTime::create_now_local().format("%Y-%m-%d %H:%M:%S") << "\n";

    ticket_config << "No. Ticket: 0001"
                  << "\n\n";

    if (v_check_config[4]->get_active())
        ticket_config << std::left << std::setw(10) << "Le atendio: "
                      << "Juan Perez \n\n"
                      << "--------------------------------\n";

    ticket_config << "Articulo\n"
                  << std::left << std::setw(10) << "Cnt." << std::setw(10) << "P.U."
                  << "T.\n"
                  << "--------------------------------\n";
    ticket_config << "Articulo de prueba\n"
                  << std::setw(10) << "10" << std::setw(10) << "6"
                  << std::setw(10) << "60"
                  << "\n"
                  << "--------------------------------\n";
    ticket_config << std::left << std::setw(20) << "Total:"
                  << "60.00\n";
    ticket_config << std::left << std::setw(20) << "Tipo de Pago:"
                  << "Efectivo\n"
                  << "--------------------------------\n";
    ticket_config << std::left << std::setw(20) << "Ingreso:"
                  << "90.00\n";
    ticket_config << std::left << std::setw(20) << "Cambio:"
                  << "30.00\n"
                  << "--------------------------------\n";

    if (v_check_config[5]->get_active())
        ticket_config << "**" << db_empresa->get_item(3)->m_valor << "**"
                      << "\n"
                      << "--------------------------------\n";
    if (v_check_config[0]->get_active())
        ticket_config << "**" << db_empresa->get_item(4)->m_valor << "**"
                      << "\n"
                      << "--------------------------------\n";

    return ticket_config.str();
}

void Impresora::actualiza_buffer()
{
    text_buffer->set_text(test_text_impresion());
}

namespace Global
{
    namespace System
    {
        std::string imprime_ticket(Glib::RefPtr<MLog> log, int faltante)
        {
            std::stringstream ticket_config;
            auto db = std::make_unique<Configuracion>();
            auto db_empresa = db->get_conf_data(10, 14);

            ticket_config << "****** "<< log->m_tipo <<"******\n"
                        << "--------------------------------\n\n"
                        << std::left << std::setw(20) << db_empresa->get_item(0)->m_valor << "\n\n";

            if (Global::Widget::Impresora::state_vizualizacion[0])
                ticket_config << "Direccion: " << db_empresa->get_item(1)->m_valor << "\n"
                            << "--------------------------------\n";

            if (Global::Widget::Impresora::state_vizualizacion[1])
                ticket_config << "RFC: " << db_empresa->get_item(2)->m_valor << "\n"
                            << "--------------------------------\n";

            if (Global::Widget::Impresora::state_vizualizacion[2])
                ticket_config << "Fecha: " << Glib::DateTime::create_now_local().format("%Y-%m-%d %H:%M:%S") << "\n";

            ticket_config << "No. Ticket: " << log->m_id << "\n\n";

            if (Global::Widget::Impresora::state_vizualizacion[3])
                ticket_config << std::left << std::setw(10) << "Le atendio: " << Global::User::Current << "\n\n"
                            << "--------------------------------\n";

            ticket_config << std::left << std::setw(20) << "Total:" << log->m_total <<"\n";
            ticket_config << std::left << std::setw(20) << "Tipo de Pago:" << "Efectivo\n"
                        << "--------------------------------\n";
            ticket_config << std::left << std::setw(20) << "Ingreso:" << log->m_ingreso <<"\n";
            ticket_config << std::left << std::setw(20) << "Cambio:" << log->m_cambio <<"\n";
            ticket_config << std::left << std::setw(20) << "Faltante:" << faltante <<"\n";
            ticket_config << std::left << std::setw(20) << "Faltante Cambio:"<< Pago::faltante << "\n"
                        << "-------------STATUS------------\n"
                        << log->m_estatus <<"\n"
                        << "--------------------------------\n";

            if (Global::Widget::Impresora::state_vizualizacion[4])
                ticket_config << "**" << db_empresa->get_item(3)->m_valor << "**\n"
                            << "--------------------------------\n";

            if (Global::Widget::Impresora::state_vizualizacion[5])
                ticket_config << "**" << db_empresa->get_item(4)->m_valor << "**\n"
                            << "--------------------------------\n";

            return ticket_config.str();
        }
    }
}
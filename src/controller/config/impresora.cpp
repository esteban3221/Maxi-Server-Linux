#include "controller/config/impresora.hpp"
#include "impresora.hpp"

Impresora::Impresora(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VImpresora(cobject, refBuilder)
{

    v_list_config_visualizacion->signal_row_activated().connect(sigc::mem_fun(*this, &Impresora::activa_visualizacion));
    v_list_config->signal_row_activated().connect(sigc::mem_fun(*this, &Impresora::activa_impresion));
    v_list_config_test->signal_row_activated().connect(sigc::mem_fun(*this, &Impresora::activa_test));

    v_text_ticket->set_buffer(text_buffer);
    v_text_ticket->signal_map().connect(sigc::mem_fun(*this, &Impresora::actualiza_buffer));
    v_text_ticket->signal_map().connect(sigc::mem_fun(*this, &Impresora::init_data));

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
    Global::System::imprime_ticket(MLog::create(1110, 1, "Test Impresion", "Este es un ticket de prueba para verificar la funcionalidad de impresión.", 10000, 10000, 0, "Exito", Glib::DateTime::create_now_local()));
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
    std::stringstream ticket;
    auto db = std::make_unique<Configuracion>();
    auto db_empresa = db->get_conf_data(10, 14);

    ticket << "<span size=\"x-large\" weight=\"bold\">Test Impresion</span>\n"
           << std::left << std::setw(20) << "<span weight=\"bold\">Descripcion:</span>"
           << std::right << std::setw(20) << "Impresora POS/Maxicajero" << "\n";

    // Fecha (Respetando el toggle de visualización del servidor)
    if (Global::Widget::Impresora::state_vizualizacion[2])
        ticket << "<span size=\"small\">" << Glib::DateTime::create_now_local().format("%Y-%m-%d %H:%M:%S") << "</span>\n";

    ticket << "====================================\n\n";

    // Datos de la empresa
    ticket << "<span weight=\"bold\">" << db_empresa->get_item(0)->m_valor << "</span>\n";

    if (Global::Widget::Impresora::state_vizualizacion[0])
        ticket << db_empresa->get_item(1)->m_valor << "\n";

    if (Global::Widget::Impresora::state_vizualizacion[1])
        ticket << "RFC: " << db_empresa->get_item(2)->m_valor << "\n";

    ticket << "------------------------------------\n";

    // Información de la operación
    ticket << "<span weight=\"bold\">Folio:</span> 10 000" << "\n";

    if (Global::Widget::Impresora::state_vizualizacion[3])
        ticket << "<span weight=\"bold\">Atendió:</span> " << Global::User::Current << "\n";

    ticket << "------------------------------------\n";

    // Detalle de montos (alineados a la derecha)
    ticket << std::left << std::setw(18) << "Total:"
           << std::right << std::setw(18) << "<span weight=\"bold\" size=\"large\">$" << 10000 << "</span>\n";

    ticket << std::left << std::setw(18) << "Tipo de pago:"
           << std::right << std::setw(18) << "Efectivo\n";

    ticket << std::left << std::setw(18) << "Ingreso:"
           << std::right << std::setw(18) << "$" << 10000 << "\n";

    ticket << std::left << std::setw(18) << "Cambio:"
           << std::right << std::setw(18) << "$" << 0 << "\n";

    ticket << "------------------------------------\n";

    // Estado destacado con colores (Verde, Naranja, Rojo)
    ticket << "<span size=\"large\" weight=\"bold\" foreground=\""
              "#2ecc71"
           << "\">Completado</span>\n";

    ticket << "====================================\n";

    // Pie de página (Contacto y Agradecimiento)
    if (Global::Widget::Impresora::state_vizualizacion[4])
        ticket << "<span size=\"small\" style=\"italic\">"
               << db_empresa->get_item(3)->m_valor << "</span>\n";

    if (Global::Widget::Impresora::state_vizualizacion[5])
        ticket << "\n<span size=\"small\" style=\"italic\" weight=\"bold\">"
               << db_empresa->get_item(4)->m_valor << "</span>\n";

    ticket << "\n\n\n";

    return ticket.str();
}

void Impresora::actualiza_buffer()
{
    text_buffer->set_text("");
    text_buffer->insert_markup(text_buffer->end(), test_text_impresion());
}

namespace Global
{
    namespace System
    {
        void imprime_ticket(Glib::RefPtr<MLog> log)
        {
            std::stringstream ticket;
            auto db = std::make_unique<Configuracion>();
            auto db_empresa = db->get_conf_data(10, 14);

            ticket << "<span size=\"x-large\" weight=\"bold\">" << log->m_tipo << "</span>\n"
                   << std::left << std::setw(20) << "<span weight=\"bold\">Descripcion:</span>"
                   << std::right << std::setw(20) << log->m_descripcion << "\n";

            // Fecha (Respetando el toggle de visualización del servidor)
            if (Global::Widget::Impresora::state_vizualizacion[2])
                ticket << "<span size=\"small\">" << Glib::DateTime::create_now_local().format("%Y-%m-%d %H:%M:%S") << "</span>\n";

            ticket << "====================================\n\n";

            // Datos de la empresa
            ticket << "<span weight=\"bold\">" << db_empresa->get_item(0)->m_valor << "</span>\n";

            if (Global::Widget::Impresora::state_vizualizacion[0])
                ticket << db_empresa->get_item(1)->m_valor << "\n";

            if (Global::Widget::Impresora::state_vizualizacion[1])
                ticket << "RFC: " << db_empresa->get_item(2)->m_valor << "\n";

            ticket << "------------------------------------\n";

            // Información de la operación
            ticket << "<span weight=\"bold\">Folio:</span> " << log->m_id << "\n";

            if (Global::Widget::Impresora::state_vizualizacion[3])
                ticket << "<span weight=\"bold\">Atendió:</span> " << Global::User::Current << "\n";

            ticket << "------------------------------------\n";

            // Detalle de montos (alineados a la derecha)
            ticket << std::left << std::setw(18) << "Total:"
                   << std::right << std::setw(18) << "<span weight=\"bold\" size=\"large\">$" << log->m_total << "</span>\n";

            ticket << std::left << std::setw(18) << "Tipo de pago:"
                   << std::right << std::setw(18) << "Efectivo\n";

            ticket << std::left << std::setw(18) << "Ingreso:"
                   << std::right << std::setw(18) << "$" << log->m_ingreso << "\n";

            ticket << std::left << std::setw(18) << "Cambio:"
                   << std::right << std::setw(18) << "$" << log->m_cambio << "\n";

            ticket << "------------------------------------\n";

            // Estado destacado con colores (Verde, Naranja, Rojo)
            ticket << "<span size=\"large\" weight=\"bold\" foreground=\""
                   << (log->m_estatus == "Exito" ? "#2ecc71" : (log->m_estatus == "PENDIENTE" ? "#f39c12" : "#e74c3c"))
                   << "\">" << log->m_estatus << "</span>\n";

            ticket << "====================================\n";

            // Pie de página (Contacto y Agradecimiento)
            if (Global::Widget::Impresora::state_vizualizacion[4])
                ticket << "<span size=\"small\" style=\"italic\">"
                       << db_empresa->get_item(3)->m_valor << "</span>\n";

            if (Global::Widget::Impresora::state_vizualizacion[5])
                ticket << "\n<span size=\"small\" style=\"italic\" weight=\"bold\">"
                       << db_empresa->get_item(4)->m_valor << "</span>\n";

            ticket << "\n\n\n";

            auto print_op = PrintFormOperation::create();
            auto settings = Gtk::PrintSettings::create();
            auto page_setup = Gtk::PageSetup::create();

            print_op->set_markup(ticket.str());
            print_op->set_track_print_status(true);

            // Tamaño típico para ticket (80mm ancho × longitud variable)
            auto paper_size = Gtk::PaperSize("custom", "Ticket", 80, 297, Gtk::Unit::MM); // 80mm × 297mm (largo suficiente)
            page_setup->set_paper_size(paper_size);
            page_setup->set_orientation(Gtk::PageOrientation::PORTRAIT);
            page_setup->set_top_margin(5, Gtk::Unit::MM);
            page_setup->set_bottom_margin(5, Gtk::Unit::MM);
            page_setup->set_left_margin(5, Gtk::Unit::MM);
            page_setup->set_right_margin(5, Gtk::Unit::MM);

            print_op->set_default_page_setup(page_setup);
            print_op->set_print_settings(settings);

            try
            {
                print_op->run(Gtk::PrintOperation::Action::PRINT, *Global::Widget::v_main_window);

                print_op->signal_done().connect([](Gtk::PrintOperation::Result result)
                                                {
                    switch (result)
                    {
                        case Gtk::PrintOperation::Result(0):
                            Global::System::showNotify("Error al imprimir", "Ocurrió un error durante la impresión.", "dialog-error");
                            break;
                        case Gtk::PrintOperation::Result::APPLY:
                            Global::System::showNotify("Ticket impreso correctamente", "El ticket se imprimió correctamente.", "dialog-information");
                            break;
                        case Gtk::PrintOperation::Result::CANCEL:
                            Global::System::showNotify("Impresión cancelada", "La impresión fue cancelada.", "dialog-warning");
                            break;
                        default:
                            break;
                    } });
            }
            catch (const Gtk::PrintError &ex)
            {
                Global::System::showNotify("Error al imprimir", ex.what(), "dialog-error");
                std::cerr << "Error en impresión: " << ex.what() << std::endl;
            }
        }
    }
}
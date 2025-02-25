#include "controller/config/general.hpp"
#include "general.hpp"

General::General(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VGeneral(cobject, refBuilder)
{
    //@@@ por el momento no se tiene una vista de carrousel
    v_Drop_temporizador->set_sensitive(false);
    v_btn_select_carrousel->set_sensitive(false);

    carga_estado_inicial();
    v_list_configurable->signal_row_activated().connect(sigc::mem_fun(*this, &General::on_list_checked));
    v_list_config_general->signal_row_activated().connect(sigc::mem_fun(*this, &General::on_list_activate));

    v_btn_select_icon->signal_clicked().connect(sigc::mem_fun(*this, &General::on_button_file_clicked));
    v_ety_mensaje_inicio->signal_changed().connect(sigc::mem_fun(*this, &General::on_ety_changed));
}

General::~General()
{
}

void General::carga_estado_inicial()
{
    auto db = std::make_unique<Configuracion>();
    auto db_general = db->get_conf_data(5, 9);

    v_check_config_notifi->set_active(db_general->get_item(0)->m_valor == "1");
    v_lbl_path_icon->set_text(db_general->get_item(1)->m_valor);
    v_ety_mensaje_inicio->set_text(db_general->get_item(2)->m_valor);
    v_lbl_path_carrousel->set_text(db_general->get_item(3)->m_valor);
    v_Drop_temporizador->set_selected(std::stoi(db_general->get_item(4)->m_valor));
}

void General::on_list_activate(Gtk::ListBoxRow *row)
{
    switch (row->get_index())
    {
    case 0:
    {
        // llamar vista para establecer un nuevo nip
        // main_stack.set_visible_child(*nip_set_view);
        break;
    }
    case 1:
    {
        // reiniciar
        on_click_reboot();
        break;
    }
    case 2:
    {
        // apagar
        on_click_shutdown();
        break;
    }
    case 3:
    {
        // restablecer de fabrica
        on_rest_app();
        break;
    }
    default:
        break;
    }
}

void General::on_list_checked(Gtk::ListBoxRow *row)
{
    if (row->get_index() == 0)
        if (v_check_config_notifi->get_active())
            v_check_config_notifi->set_active(false);
        else
            v_check_config_notifi->set_active();

    auto db = std::make_unique<Configuracion>();
    auto check = MConfiguracion::create(5, "Mostrar Notificacion", std::to_string(v_check_config_notifi->get_active()));
    db->update_conf(check);
}

void General::on_click_reboot()
{
    v_dialog.reset(new Gtk::MessageDialog(*Global::Widget::v_main_window, "Reinicio", false, Gtk::MessageType::QUESTION, Gtk::ButtonsType::CANCEL, true));
    v_dialog->add_button("Reiniciar", Gtk::ResponseType::OK)->add_css_class({"warning"});
    v_dialog->set_secondary_text("¿Desea reinicia el dispositivo?");
    v_dialog->signal_response().connect([this](int response)
                                        {
        if(Gtk::ResponseType::OK == response)
            Global::System::exec("shutdown -r");

        v_dialog->close(); });

    v_dialog->show();
}
void General::on_click_shutdown()
{
    v_dialog.reset(new Gtk::MessageDialog(*Global::Widget::v_main_window, "Apagar", false, Gtk::MessageType::QUESTION, Gtk::ButtonsType::CANCEL, true));
    v_dialog->add_button("Apagar", Gtk::ResponseType::OK)->add_css_class({"warning"});
    v_dialog->set_secondary_text("¿Desea Apagar el dispositivo?");
    v_dialog->signal_response().connect([this](int response)
                                        {
        if(Gtk::ResponseType::OK == response)
            Global::System::exec("shutdown +1");

        v_dialog->close(); });

    v_dialog->show();
}

void General::on_rest_app()
{
    v_dialog.reset(new Gtk::MessageDialog(*Global::Widget::v_main_window, "Reinicio de Fabrica", false, Gtk::MessageType::QUESTION, Gtk::ButtonsType::CANCEL, true));
    auto btn = v_dialog->add_button("Ok", Gtk::ResponseType::OK);
    btn->add_css_class({"destructive-action"});
    btn->set_sensitive(false);
    v_dialog->set_secondary_text("Esto eliminara toda configuración y datos guardados.\n"
                                 "¿Desea continuar?");

    auto chkb = Gtk::manage(new Gtk::CheckButton("Estoy consiente de la perdida completa de los datos del dispositivo."));
    v_dialog->get_content_area()->append(*chkb);

    chkb->signal_toggled().connect([btn, chkb]()
                                   { btn->set_sensitive(chkb->get_active()); });

    v_dialog->signal_response().connect([this](int response)
                                        {
        if(Gtk::ResponseType::OK == response)
            std::cout << "Reinicio de Fabrica\n";

        v_dialog->close(); });

    v_dialog->show();
}

void General::on_button_file_clicked()
{
    auto dialog = new Gtk::FileChooserDialog(*Global::Widget::v_main_window,
                                             /*type ? "Escoge un Directorio" : */ "Escoge una Imágen",
                                             /*type ? Gtk::FileChooser::Action::SELECT_FOLDER :*/ Gtk::FileChooser::Action::OPEN,
                                             true);

    dialog->set_modal(true);
    dialog->set_default_size(10, 10);

    dialog->signal_response().connect(sigc::bind(sigc::mem_fun(*this, &General::on_file_dialog_response), dialog));

    dialog->add_button("_Cancel", Gtk::ResponseType::CANCEL);
    dialog->add_button("_Open", Gtk::ResponseType::OK);

    auto filter_images = Gtk::FileFilter::create();
    filter_images->set_name("Image files");
    filter_images->add_mime_type("image/*");
    dialog->add_filter(filter_images);
    dialog->set_filter(filter_images);

    auto path = Glib::get_user_special_dir(Glib::UserDirectory::DOCUMENTS);
    auto doc = Gio::File::create_for_path(path + "/img");
    dialog->set_current_folder(doc);

    dialog->show();
}

void General::on_file_dialog_response(int response_id, Gtk::FileChooserDialog *dialog)
{
    switch (response_id)
    {
    case Gtk::ResponseType::OK:
    {
        std::cout << "Open clicked." << std::endl;
        auto filename = dialog->get_file()->get_path();
        v_lbl_path_icon->set_text(filename);

        auto db = std::make_unique<Configuracion>();
        auto data = MConfiguracion::create(6, "Ruta logo incio", filename);
        db->update_conf(data);

        break;
    }
    case Gtk::ResponseType::CANCEL:
    {
        std::cout << "Cancel clicked." << std::endl;
        break;
    }
    default:
    {
        std::cout << "Unexpected button clicked." << std::endl;
        break;
    }
    }
    delete dialog;
}

void General::on_ety_changed()
{
    auto db = std::make_unique<Configuracion>();
    auto data = MConfiguracion::create(7, "Mensaje Inicio", v_ety_mensaje_inicio->get_text());
    db->update_conf(data);
}

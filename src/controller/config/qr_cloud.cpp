#include "controller/config/qr_cloud.hpp"

QrCloud::QrCloud(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder)
    : VQrCloud(cobject, refBuilder)
{
    signal_map().connect(sigc::mem_fun(*this, &QrCloud::on_show_map));
    signal_unmap().connect([]()
                           { 
        g_info("Cerrando polleo de status");
        Cloud::poll_status_pair.store(false); });
    v_dropdown_canal->property_selected().signal_changed().connect(sigc::mem_fun(*this, &QrCloud::on_dropdown_canal_changed));
}

QrCloud::~QrCloud()
{
}

void QrCloud::on_dropdown_canal_changed()
{
    /*
            model: Gtk.StringList {
                strings [
                  "LTS (Estable)",
                  "Test (Pruebas)"
                ]
              };
    */
    auto selected_index = v_dropdown_canal->get_selected();
    auto db = std::make_unique<Configuracion>();
    std::string canal = (selected_index == 0) ? "lts" : "test";

    db->update_conf(MConfiguracion::create(105, "Canal de Actualización", canal));
}

void QrCloud::on_show_map(void)
{
    Cloud::poll_status_pair.store(true);
    {
        std::lock_guard<std::mutex> lock(Cloud::cloud_mutex);
        Cloud::qr_url.clear();
        Cloud::pairing_code.clear();
    }

    auto db = std::make_unique<Configuracion>();

    auto config_data = db->get_conf_data(101, 105);
    if (config_data && !config_data->get_item(2)->m_valor.empty())
    {
        std::string device_uuid = config_data->get_item(2)->m_valor;
        v_label_uuid->set_text(device_uuid);
    }
    else
        v_label_uuid->set_text("No encontrado");

    int channel_index = config_data->get_item(4)->m_valor == "lts" ? 0 : 1;
    v_dropdown_canal->set_selected(channel_index);

    if (config_data && !config_data->get_item(1)->m_valor.empty() && !config_data->get_item(1)->m_valor.empty())
    {
        v_label_status->set_text("Dispositivo ya vinculado 🟢");
        v_label_pin->set_text("N/A (Vinculado)");
        return;
    }

    if (!config_data || config_data->get_item(0)->m_valor.empty())
    {
        v_label_status->set_text("URL de nube no configurada");
        return;
    }
    std::string server_url = config_data->get_item(0)->m_valor;

    v_label_status->set_text("Conectando con la nube...");

    std::thread([this, server_url]()
                {
        bool is_success = iniciar_vinculacion_qr(server_url);

        Glib::signal_idle().connect_once([this, is_success]()
        {
            if (is_success) 
            {
                v_label_status->set_text("¡Vinculado y Conectado con éxito! 🟢");
                Global::System::showNotify("Maxicajero", "¡Vinculación Exitosa!", "dialog-information");
            } else 
                v_label_status->set_text("Vinculación detenida o expirada 🔴");
        }); })
        .detach();
}
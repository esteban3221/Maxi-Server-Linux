#include "controller/config/qr_cloud.hpp"

QrCloud::QrCloud()
{
    signal_map().connect(sigc::mem_fun(*this, &QrCloud::on_show_map));
    signal_unmap().connect([]()
                           { 
        g_info("Cerrando polleo de status");
        Cloud::poll_status_pair.store(false); });
}

QrCloud::~QrCloud()
{
}

void QrCloud::on_show_map(void)
{
    Cloud::poll_status_pair.store(true);
    {
        std::lock_guard<std::mutex> lock(Cloud::cloud_mutex);
        Cloud::qr_url.clear();
        Cloud::pairing_code.clear();
    }

    auto &database = Database::getInstance();

    auto config_uuid = database.sqlite3->command("SELECT valor FROM configuracion WHERE id = 103");
    if (config_uuid && !config_uuid->at("valor").empty())
    {
        std::string device_uuid = config_uuid->at("valor")[0];
        v_label_uuid.set_text("UUID: " + device_uuid);
    }
    else
    {
        v_label_uuid.set_text("UUID: No encontrado");
    }

    auto config_key = database.sqlite3->command("SELECT valor FROM configuracion WHERE id = 102");
    if (config_key && !config_key->at("valor").empty() && !config_key->at("valor")[0].empty())
    {
        v_label_status.set_text("Estado: Dispositivo ya vinculado 🟢");
        v_label_pin.set_text("PIN: N/A (Vinculado)");
        return; // Salimos de inmediato porque no se necesita QR
    }

    auto config_url = database.sqlite3->command("SELECT valor FROM configuracion WHERE id = 101");
    if (!config_url || config_url->at("valor").empty())
    {
        v_label_status.set_text("Estado: URL de nube no configurada");
        return;
    }
    std::string server_url = config_url->at("valor")[0];

    v_label_status.set_text("Estado: Conectando con la nube...");

    std::thread([this, server_url]()
                {
                    auto &db = Database::getInstance();

                    auto res_uuid = db.sqlite3->command("SELECT valor FROM configuracion WHERE id = 103");
                    std::string device_uuid;
                    if (res_uuid && !res_uuid->at("valor").empty() && !res_uuid->at("valor")[0].empty())
                    {
                        device_uuid = res_uuid->at("valor")[0];
                    }
                    else
                    {
                        device_uuid = "CAJERO-" + std::string(std::unique_ptr<gchar, void (*)(gpointer)>(g_uuid_string_random(), g_free).get());
                        db.sqlite3->command("UPDATE configuracion SET valor = ? WHERE id = 103", device_uuid.c_str());
                    }

                    crow::json::wvalue payload;
                    payload["uuid"] = device_uuid;

                    auto response = cpr::Post(
                        cpr::Url{server_url + "/api/devices/init-pair"},
                        cpr::Header{{"Content-Type", "application/json"}},
                        cpr::Body{payload.dump()},
                        cpr::Timeout{5000});

                    if (response.status_code != 200 && response.status_code != 201)
                    {
                        g_warning("Error al iniciar vinculación (%d): %s", response.status_code, response.text.c_str());
                        Glib::signal_idle().connect_once([this]()
                                                         {
                v_label_status.set_text("Estado: Error de conexión con la nube 🔴");
                Global::System::showNotify("Maxicajero", "No es posible conectar con la nube", "dialog-error"); });
                        return;
                    }

                    std::string local_qr, local_pin;
                    {
                        std::lock_guard<std::mutex> lock(Cloud::cloud_mutex);
                        auto json_res = crow::json::load(response.text);
                        Cloud::pairing_code = json_res["pairingCode"].s();
                        Cloud::qr_url = json_res["qrUrl"].s();

                        local_qr = Cloud::qr_url;
                        local_pin = Cloud::pairing_code;
                    }

                    Glib::signal_idle().connect_once([this, local_qr, local_pin]()
                                                     {
            actualizar_qr(local_qr);
            v_label_pin.set_text("PIN: " + local_pin);
            v_label_status.set_text("Estado: Esperando vinculación en celular..."); });

                    bool is_success = false;
                    while (Cloud::poll_status_pair.load())
                    {
                        std::this_thread::sleep_for(std::chrono::seconds(3));

                        auto poll_res = cpr::Get(
                            cpr::Url{server_url + "/api/devices/pair-status/" + local_pin},
                            cpr::Timeout{3000});

                        if (poll_res.status_code == 200)
                        {
                            auto poll_json = crow::json::load(poll_res.text);
                            std::string status = poll_json["status"].s();

                            if (status == "CLAIMED")
                            {
                                std::string api_key = poll_json["apiKey"].s();

                                db.sqlite3->command("UPDATE configuracion SET valor = ? WHERE id = 101", server_url.c_str());
                                db.sqlite3->command("UPDATE configuracion SET valor = ? WHERE id = 102", api_key.c_str());

                                is_success = true;
                                break;
                            }
                            else if (status == "EXPIRED")
                            {
                                g_warning("El código de vinculación expiró.");
                                break;
                            }
                        }
                    }

                    Glib::signal_idle().connect_once([this, is_success]()
                                                     {
            if (is_success) {
                v_label_status.set_text("Estado: ¡Vinculado y Conectado con éxito! 🟢");
                Global::System::showNotify("Maxicajero", "¡Vinculación Exitosa!", "dialog-information");
            } else {
                v_label_status.set_text("Estado: Vinculación detenida o expirada 🔴");
            } }); })
        .detach();
}
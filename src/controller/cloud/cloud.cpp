#include "cloud.h"

Cloud::Cloud()
{
    poll_status_pair.store(false);
}

Cloud::~Cloud()
{
}

crow::json::wvalue Cloud::json_cloud(Glib::RefPtr<MLog> t_log, const std::string &device_uuid)
{
    crow::json::wvalue data;

    data["logData"]["uuidCloud"] = t_log->m_uuid_cloud.raw();
    data["logData"]["localId"] = t_log->m_id;
    data["logData"]["idUserLocal"] = t_log->m_id_user; // Ajustado al nombre de Prisma
    data["logData"]["tipo"] = t_log->m_tipo.raw();
    data["logData"]["descripcion"] = t_log->m_descripcion.raw();
    data["logData"]["ingreso"] = t_log->m_ingreso;
    data["logData"]["cambio"] = t_log->m_cambio;
    data["logData"]["total"] = t_log->m_total;
    data["logData"]["estatus"] = t_log->m_estatus.raw();
    data["logData"]["fecha"] = t_log->m_fecha.format_iso8601().raw();

    return data;
}

const std::shared_ptr<ResultMap> Cloud::get_logs_pendientes_sincronizar(int limite)
{
    auto &database = Database::getInstance();
    std::string query = "SELECT * FROM log WHERE sync_status = 0 ORDER BY Id ASC LIMIT ?";
    return database.sqlite3->command(query, limite);
}

void Cloud::marcar_como_sincronizado(size_t id_log, const std::string &uuid_cloud)
{
    auto &database = Database::getInstance();
    database.sqlite3->command(
        "UPDATE log SET sync_status = 1, uuid_cloud = ? WHERE Id = ?",
        uuid_cloud.c_str(),
        id_log);
}

void Cloud::sincronizar_con_nube_async(Glib::RefPtr<MLog> t_log)
{
    auto &database = Database::getInstance();

    auto config_url = database.sqlite3->command("SELECT valor FROM configuracion WHERE id = 101");
    auto config_key = database.sqlite3->command("SELECT valor FROM configuracion WHERE id = 102");
    auto config_uuid = database.sqlite3->command("SELECT valor FROM configuracion WHERE id = 103");

    if (!config_url || config_url->empty())
        return;

    std::string server_url = config_url->at("valor")[0];
    std::string api_key = config_key->at("valor")[0];
    std::string device_uuid = config_uuid->at("valor")[0];

    crow::json::wvalue payload = json_cloud(t_log, device_uuid);

    auto request_async = cpr::PostAsync(
        cpr::Url{server_url + "/api/logs"},
        cpr::Header{
            {"Content-Type", "application/json"},
            {"x-api-key", api_key}},
        cpr::Body{payload.dump()},
        cpr::Timeout{3000});

    std::thread([async = std::move(request_async), id = t_log->m_id, uuid = t_log->m_uuid_cloud.raw()]() mutable
                {
                    auto r = async.get();
                    if (r.error)
                    {
                        g_warning("Error al sincronizar con la nube: %d - %s", (uint16_t)r.error.code, r.error.message);
                        return;
                    }

                    if (r.status_code == 200 || r.status_code == 201)
                        Cloud::marcar_como_sincronizado(id, uuid);
                    else
                        g_warning("Error al sincronizar con la nube, código de estado: %d", r.status_code); })
        .detach();
}

bool registrar_dispositivo_en_nube(const std::string &server_url,
                                   const std::string &master_key,
                                   const std::string &nombre_sucursal)
{
    auto &database = Database::getInstance();
    auto config_key = database.sqlite3->command("SELECT valor FROM configuracion WHERE id = 102");

    if (config_key && !config_key->at("valor").empty() && !config_key->at("valor")[0].empty())
    {
        g_info("El cajero ya cuenta con API Key registrada.");
        return true;
    }

    auto config_uuid = database.sqlite3->command("SELECT valor FROM configuracion WHERE id = 103");
    std::string device_uuid;

    if (config_uuid && !config_uuid->at("valor").empty() && !config_uuid->at("valor")[0].empty())
        device_uuid = config_uuid->at("valor")[0];
    else
        device_uuid = "CAJERO-" + std::string(std::unique_ptr<gchar, void (*)(gpointer)>(g_uuid_string_random(), g_free).get());

    crow::json::wvalue payload;
    payload["uuid"] = device_uuid;
    payload["name"] = nombre_sucursal;

    auto response = cpr::Post(
        cpr::Url{server_url + "/api/devices/register"},
        cpr::Header{
            {"Content-Type", "application/json"},
            {"x-master-key", master_key}},
        cpr::Body{payload.dump()},
        cpr::Timeout{5000});

    if (response.status_code == 201 || response.status_code == 409)
    {
        auto json_res = crow::json::load(response.text);

        if (response.status_code == 201)
        {
            std::string api_key_generada = json_res["device"]["apiKey"].s();

            database.sqlite3->command("UPDATE configuracion SET valor = ? WHERE id = 101", server_url.c_str());
            database.sqlite3->command("UPDATE configuracion SET valor = ? WHERE id = 102", api_key_generada.c_str());
            database.sqlite3->command("UPDATE configuracion SET valor = ? WHERE id = 103", device_uuid.c_str());

            g_info("¡Vinculación exitosa! API Key asignada: %s", api_key_generada);
            return true;
        }
        else
        {
            g_error("El dispositivo ya existía en la nube. Verifique su UUID.");
            return false;
        }
    }

    g_error("Error de registro (%d): %s", response.status_code, response.text);
    return false;
}

#include <thread>
#include <chrono>

bool iniciar_vinculacion_qr(const std::string &server_url)
{
    auto &database = Database::getInstance();

    auto config_key = database.sqlite3->command("SELECT valor FROM configuracion WHERE id = 102");
    if (config_key && !config_key->at("valor").empty() && !config_key->at("valor")[0].empty())
    {
        g_info("El cajero ya se encuentra vinculado.");
        return true;
    }

    auto config_uuid = database.sqlite3->command("SELECT valor FROM configuracion WHERE id = 103");
    std::string device_uuid;

    if (config_uuid && !config_uuid->at("valor").empty() && !config_uuid->at("valor")[0].empty())
        device_uuid = config_uuid->at("valor")[0];
    else
    {
        device_uuid = "CAJERO-" + std::string(std::unique_ptr<gchar, void (*)(gpointer)>(g_uuid_string_random(), g_free).get());
        database.sqlite3->command("UPDATE configuracion SET valor = ? WHERE id = 103", device_uuid.c_str());
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
        g_error("Error al iniciar vinculación  (%d): %s", response.status_code, response.text);
        return false;
    }

    std::lock_guard<std::mutex> lock(Cloud::cloud_mutex);
    auto json_res = crow::json::load(response.text);
    Cloud::pairing_code = json_res["pairingCode"].s();
    Cloud::qr_url = json_res["qrUrl"].s();

    std::cout << "\n=======================================================" << std::endl;
    std::cout << " CÓDIGO DE VINCULACIÓN: " << Cloud::pairing_code << std::endl;
    std::cout << " Escanea o ingresa este enlace en el Dashboard Web:" << std::endl;
    std::cout << " " << Cloud::qr_url << std::endl;
    std::cout << "=======================================================\n"
              << std::endl;

    g_info("Esperando confirmación desde la Web...");
    while (Cloud::poll_status_pair.load())
    {
        std::this_thread::sleep_for(std::chrono::seconds(3));

        auto poll_res = cpr::Get(
            cpr::Url{server_url + "/api/devices/pair-status/" + Cloud::pairing_code},
            cpr::Timeout{3000});

        if (poll_res.status_code == 200)
        {
            auto poll_json = crow::json::load(poll_res.text);
            std::string status = poll_json["status"].s();

            if (status == "CLAIMED")
            {
                std::string api_key = poll_json["apiKey"].s();

                database.sqlite3->command("UPDATE configuracion SET valor = ? WHERE id = 101", server_url.c_str());
                database.sqlite3->command("UPDATE configuracion SET valor = ? WHERE id = 102", api_key.c_str());

                g_info("¡Dispositivo vinculado con éxito desde la Web!");
                return true;
            }
            else if (status == "EXPIRED")
            {
                g_warning("El código expiró. Reiniciando vinculación...");
                return iniciar_vinculacion_qr(server_url);
            }
        }
    }

    return false;
}
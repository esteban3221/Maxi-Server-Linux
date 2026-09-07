#pragma once
#include <cpr/cpr.h>
#include <crow.h>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <string_view>
#include <unistd.h>
#include <glibmm.h>

#include "config/version.hpp"
#include "coneccion.hpp"

inline void checkAndApplyUpdate()
{
    auto &database = Database::getInstance();
    auto result = database.sqlite3->command("SELECT valor FROM configuracion WHERE id = 101 OR id = 105 ORDER BY id ASC");
    const std::string CHANNEL = result->at("valor")[1];
    const std::string URL = result->at("valor")[0];
    std::string url_manifest = URL + "/updates/" + CHANNEL + "/latest.json";

    auto response = cpr::Get(cpr::Url{url_manifest});
    if (response.status_code != 200)
    {
        g_warning("No se pudo contactar al servidor de actualizaciones.");
        return;
    }

    auto json_data = crow::json::load(response.text);
    int remote_major = json_data["major"].i();
    int remote_minor = json_data["minor"].i();
    int remote_patch = json_data["patch"].i();
    int remote_build = json_data["build"].i();
    std::string download_url = json_data["url"].s();

    bool hay_actualizacion = false;
    if (remote_major > Maxicajero::Version::MAJOR)
        hay_actualizacion = true;
    else if (remote_major == Maxicajero::Version::MAJOR && remote_minor > Maxicajero::Version::MINOR)
        hay_actualizacion = true;
    else if (remote_major == Maxicajero::Version::MAJOR && remote_minor == Maxicajero::Version::MINOR && remote_patch > Maxicajero::Version::PATCH)
        hay_actualizacion = true;
    else if (remote_major == Maxicajero::Version::MAJOR && remote_minor == Maxicajero::Version::MINOR && remote_patch == Maxicajero::Version::PATCH && remote_build > Maxicajero::Version::BUILD)
        hay_actualizacion = true;

    if (!hay_actualizacion)
    {
        // g_info("El sistema está actualizado.");
        return;
    }

    g_info("Nueva versión encontrada. Descargando...");
    std::string temp_path = "/tmp/Maxicajero-Server-new";
    cpr::Session session;
    session.SetUrl(cpr::Url{download_url});
    std::ofstream file(temp_path, std::ios::binary);
    session.SetWriteCallback(cpr::WriteCallback{[&file](std::string_view data, intptr_t) -> bool
                                                {
                                                    file.write(data.data(), static_cast<std::streamsize>(data.size()));
                                                    return file.good();
                                                }});
    auto download_response = session.Get();

    if (download_response.status_code != 200)
    {
        g_warning("Error al descargar la actualización.");
        return;
    }
    file.close();

    std::string chmod_cmd = "chmod +x " + temp_path;
    system(chmod_cmd.c_str());

    char result_[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result_, PATH_MAX);
    std::string current_executable_path(result_, (count > 0) ? count : 0);

    if (!current_executable_path.empty())
    {
        if (rename(temp_path.c_str(), current_executable_path.c_str()) == 0)
        {
            g_info("Actualización aplicada con éxito. Reiniciando servicio...");
            // std::exit(0);
        }
        else
            g_warning("Error al reemplazar el binario actual.");
    }
}
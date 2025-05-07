#include "controller/configuracion.hpp"
#include "configuracion.hpp"

CConfiguracion::CConfiguracion(/* args */)
{
    CROW_ROUTE(RestApp::app, "/configuracion/actualiza_impresion").methods("POST"_method)(sigc::mem_fun(*this, &CConfiguracion::actualiza_impresion));
    CROW_ROUTE(RestApp::app, "/configuracion/actualiza_informacion_empresa").methods("POST"_method)(sigc::mem_fun(*this, &CConfiguracion::actualiza_informacion_empresa));

    CROW_ROUTE(RestApp::app, "/configuracion/get_informacion_empresa").methods("GET"_method)(sigc::mem_fun(*this, &CConfiguracion::get_informacion_empresa));
    CROW_ROUTE(RestApp::app, "/configuracion/get_informacion_sistema").methods("GET"_method)(sigc::mem_fun(*this, &CConfiguracion::get_informacion_sistema));
    CROW_ROUTE(RestApp::app, "/configuracion/get_informacion_impresora").methods("GET"_method)(sigc::mem_fun(*this, &CConfiguracion::get_informacion_impresora));
    CROW_ROUTE(RestApp::app, "/configuracion/test_impresion").methods("GET"_method)(sigc::mem_fun(*this, &CConfiguracion::test_impresion));
    CROW_ROUTE(RestApp::app, "/configuracion/reiniciar").methods("GET"_method)(sigc::mem_fun(*this, &CConfiguracion::reiniciar));
    CROW_ROUTE(RestApp::app, "/configuracion/apagar").methods("GET"_method)(sigc::mem_fun(*this, &CConfiguracion::apagar));

    CROW_ROUTE(RestApp::app, "/configuracion/custom_command").methods("POST"_method)(sigc::mem_fun(*this, &CConfiguracion::custom_command));
    CROW_ROUTE(RestApp::app, "/configuracion/actualiza_pos").methods("POST"_method)(sigc::mem_fun(*this, &CConfiguracion::actualiza_pos));
}

CConfiguracion::~CConfiguracion()
{
}

crow::response CConfiguracion::actualiza_impresion(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Cambio_M);
    try
    {
        auto db = std::make_unique<Configuracion>();

        auto bodyParams = crow::json::load(req.body);

        db->update_conf(MConfiguracion::create(15, "Activa", std::to_string(bodyParams["activa"].b())));

        db->update_conf(MConfiguracion::create(16, "agradecimiento", std::to_string(bodyParams["agradecimiento"].b())));
        db->update_conf(MConfiguracion::create(17, "fecha", std::to_string(bodyParams["fecha"].b())));
        db->update_conf(MConfiguracion::create(18, "direccion", std::to_string(bodyParams["direccion"].b())));
        db->update_conf(MConfiguracion::create(19, "rfc", std::to_string(bodyParams["rfc"].b())));
        db->update_conf(MConfiguracion::create(20, "vendedor", std::to_string(bodyParams["vendedor"].b())));
        db->update_conf(MConfiguracion::create(21, "contacto", std::to_string(bodyParams["contacto"].b())));

        return crow::response();
    }
    catch (const std::exception &e)
    {
        return crow::response(crow::CONFLICT, e.what());
    }
}

crow::response CConfiguracion::actualiza_informacion_empresa(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Cambio_M);
    try
    {

        auto bodyParams = crow::json::load(req.body);

        auto db = std::make_unique<Configuracion>();

        const std::string a[5] =
            {
                bodyParams["razon_social"].s(),
                bodyParams["direccion"].s(),
                bodyParams["rfc"].s(),
                bodyParams["contacto"].s(),
                bodyParams["agradecimiento"].s()};

        for (int i = 0; i < 5; i++)
        {
            db->update_conf(MConfiguracion::create(i + 10, "Datos Empresa", a[i]));
        }

        Global::System::showNotify("Información", "Se actualizaron datos de la empresa.", "dialog-information");

        return crow::response();
        // }
    }
    catch (const std::exception &e)
    {
        return crow::response(crow::CONFLICT, e.what());
    }
}

crow::response CConfiguracion::get_informacion_empresa(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Mostrar_Reportes);

    auto db = std::make_unique<Configuracion>();
    auto list = db->get_conf_data(10, 14);
    {
        crow::json::wvalue response_json;
        response_json["razon_social"] = list->get_item(0)->m_valor;
        response_json["direccion"] = list->get_item(1)->m_valor;
        response_json["rfc"] = list->get_item(2)->m_valor;
        response_json["contacto"] = list->get_item(3)->m_valor;
        response_json["agradecimiento"] = list->get_item(4)->m_valor;
        return crow::response(response_json);
    }
}

crow::response CConfiguracion::get_informacion_impresora(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Cambio_M);
    auto db = std::make_unique<Configuracion>();
    auto list = db->get_conf_data(15, 21);

    crow::json::wvalue response_json;
    response_json["activa"] = list->get_item(0)->m_valor == "1";

    response_json["agradecimiento"] = list->get_item(1)->m_valor == "1";
    response_json["fecha"] = list->get_item(2)->m_valor == "1";
    response_json["direccion"] = list->get_item(3)->m_valor == "1";
    response_json["rfc"] = list->get_item(4)->m_valor == "1";
    response_json["vendedor"] = list->get_item(5)->m_valor == "1";
    response_json["contacto"] = list->get_item(6)->m_valor == "1";

    return crow::response(response_json);
}

crow::response CConfiguracion::test_impresion(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Cambio_M);
    {
        std::string command = "echo \"" + Global::System::imprime_ticket(MLog::create(0, 0, "Test Impresion", 100, 0, 100, "Completado", Glib::DateTime::create_now_local()), 0) + "\" | lp";
        std::system(command.c_str());

        Global::System::showNotify("Impresión", "Se imprimio hoja de prueba.", "dialog-information");

        return crow::response(crow::status::OK);
    }
}

crow::response CConfiguracion::reiniciar(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Cambio_M);
    {
        Global::System::exec("shutdown -r +1 &");
        Global::Rest::app.stop();
        Global::System::showNotify("Sistema", "Se reiniciara el sistema en un minuto.\nTeminando procesos pendientes...\n"
                                              "Servicio de api rest esta desactivado y no recibira mas solicitudes.",
                                   "dialog-information");
        return crow::response(crow::status::OK);
    }
}

crow::response CConfiguracion::apagar(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Cambio_M);
    {
        Global::System::exec("shutdown +1 &");
        Global::Rest::app.stop();
        Global::System::showNotify("Sistema", "Se apagara el sistema en un minuto.\n"
                                              "Servicio de api rest esta desactivado y no recibira mas solicitudes.",
                                   "dialog-information");
        return crow::response(crow::status::OK);
    }
}

crow::response CConfiguracion::get_informacion_sistema(const crow::request &req)
{
    Global::Utility::valida_autorizacion(req, Global::User::Rol::Configuracion);

    const std::string &parent{"cat /sys/devices/virtual/dmi/id/"};
    const std::string &a{parent + "board_vendor"};
    const std::string &b{parent + "product_name"};

    crow::json::wvalue response_json;

    response_json["hostname"] = Global::System::exec("cat /etc/hostname");
    response_json["model"] = Global::System::exec(a.c_str()).empty() ? Global::System::exec("cat /proc/device-tree/model") : Global::System::exec(a.c_str()) + " " + Global::System::exec(b.c_str());
    response_json["processor"] = Global::System::exec("lscpu | grep -E 'Nombre del modelo|Model name' | awk -F': ' '{print $2}'");
    response_json["ram"] = Global::System::exec("grep MemTotal /proc/meminfo | awk '{print $2/1024/1024 \" GB\"}' ");
    response_json["memory"] = Global::System::exec("lsblk -o SIZE -b | head -2 | tail -1 | awk '{print $1/1024/1024/1024 \" GB\"}'");

    auto json_bill = Device::dv_bill.get_status_coneccion();
    auto json_coin = Device::dv_coin.get_status_coneccion();

    response_json["bill"] = json_bill;
    response_json["coin"] = json_coin;

    return crow::response(response_json);
}

crow::response CConfiguracion::actualiza_pos(const crow::request &req)
{
    crow::multipart::message msg(req);

    auto file_part = msg.get_part_by_name("file");
    std::string filename;

    auto content_disp = file_part.get_header_object("Content-Disposition");
    if (auto param = file_part.get_header_object("Content-Disposition").params; param.size() > 0)
        filename = param.at("filename");
    else
        filename = "archivo_sin_nombre";
    
    // Guardar el archivo en disco
    std::ofstream out(filename, std::ios::binary);
    out.write(file_part.body.data(), file_part.body.size());
    out.close();

    // quitar .zip
    std::string path = filename.substr(0, filename.find_last_of('.')).c_str();

    descomprime_zip(filename);
    int result = std::system(("sh ./" + path +"/update/install.sh").c_str());
    if (result == -1)
    {
        Global::System::showNotify("Sistema", "Error al ejecutar el script de instalación.", "dialog-error");
        throw std::runtime_error("Error al ejecutar el script de instalación.");
    }
    limpiar_archivos(filename, path);

    Global::System::showNotify("Sistema", "Actualizando Sistema no apague el sistema.", "dialog-information");

    return crow::response(200, "Archivo recibido: " + filename);
}

crow::response CConfiguracion::custom_command(const crow::request &req)
{
    auto bodyParams = crow::json::load(req.body);
    auto rol = bodyParams["rol"].i();

    Global::Utility::valida_autorizacion(req, (Global::User::Rol)rol);

    crow::json::wvalue response_json;

    crow::json::rvalue json_bill, json_coin;
    if (bodyParams.has("bill"))
    {
        auto bill_command = bodyParams["bill"]["command"].s();
        auto bill_args = bodyParams["bill"]["args"].s();
        int intentos = 5;
        Device::dv_bill.inicia_dispositivo_v6();
        json_bill = crow::json::load(Device::dv_bill.command_post(bill_command, bill_args, true).second);
    }

    if (bodyParams.has("coin"))
    {
        auto coin_command = bodyParams["coin"]["command"].s();
        auto coin_args = bodyParams["coin"]["args"].s();
        Device::dv_coin.inicia_dispositivo_v6();
        json_coin = crow::json::load(Device::dv_coin.command_post(coin_command, coin_args, true).second);
    }

    if (not json_bill.error())
        response_json["bill"] = json_bill;

    if (not json_coin.error())
        response_json["coin"] = json_coin;

    return crow::response(response_json);
}

void CConfiguracion::descomprime_zip(const std::string &filename)
{
    std::string command = "unzip -o " + filename + " -d .";
    auto resultado = std::system(command.c_str());

    if (resultado == -1)
    {
        Global::System::showNotify("Sistema", "Error al descomprimir el archivo.", "dialog-error");
        throw std::runtime_error("Error al descomprimir el archivo.");
    }

    Global::System::showNotify("Sistema", "Se descomprimio el archivo.", "dialog-information");
    
}

void CConfiguracion::limpiar_archivos(const std::string &filename, const std::string &path)
{
    std::string command2 = "rm -f " + filename + "; rm -r " + path;
    int resultado = std::system(command2.c_str());
    if (resultado == -1)
    {
        Global::System::showNotify("Sistema", "Error al eliminar el archivo comprimido.", "dialog-error");
        throw std::runtime_error("Error al eliminar el archivo comprimido.");
    }
    Global::System::showNotify("Sistema", "Se elimino el archivo comprimido.", "dialog-information");
}

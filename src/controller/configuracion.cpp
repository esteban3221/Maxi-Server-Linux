#include "controller/configuracion.hpp"

CConfiguracion::CConfiguracion(/* args */)
{
    CROW_ROUTE(RestApp::app, "/configuracion/actualiza_impresion").methods("POST"_method)(sigc::mem_fun(*this, &CConfiguracion::actualiza_impresion));
    CROW_ROUTE(RestApp::app, "/configuracion/actualiza_informacion").methods("POST"_method)(sigc::mem_fun(*this, &CConfiguracion::actualiza_informacion));

    CROW_ROUTE(RestApp::app, "/configuracion/get_informacion").methods("GET"_method)(sigc::mem_fun(*this, &CConfiguracion::get_informacion));
    CROW_ROUTE(RestApp::app, "/configuracion/get_informacion_sistema").methods("GET"_method)(sigc::mem_fun(*this, &CConfiguracion::get_informacion_sistema));
    CROW_ROUTE(RestApp::app, "/configuracion/test_impresion").methods("GET"_method)(sigc::mem_fun(*this, &CConfiguracion::test_impresion));
    CROW_ROUTE(RestApp::app, "/configuracion/reiniciar").methods("GET"_method)(sigc::mem_fun(*this, &CConfiguracion::reiniciar));
    CROW_ROUTE(RestApp::app, "/configuracion/apagar").methods("GET"_method)(sigc::mem_fun(*this, &CConfiguracion::apagar));
}

CConfiguracion::~CConfiguracion()
{
}

crow::response CConfiguracion::actualiza_impresion(const crow::request &req)
{
    try
    {
        // auto &session = app.get_context<Session>(req);
        // if (auto status = Global::User::validPermissions(req, session, Global::User::allRoles);
        //     status.first != crow::status::OK)
        // {
        //     return crow::response(status.first);
        // }

        auto bodyParams = crow::json::load(req.body);

        auto db = std::make_unique<Configuracion>();

        for (size_t i = 0; i < bodyParams.keys().size(); i++)
        {
            db->update_conf(MConfiguracion::create(i + 15, bodyParams.keys()[i], std::to_string(bodyParams[i].b())));
        }

        Global::System::showNotify("Impresion", "Se actualizo configuración de impresion.", "dialog-information");

        return crow::response();
    }
    catch (const std::exception &e)
    {
        return crow::response(crow::CONFLICT, e.what());
    }
}

crow::response CConfiguracion::actualiza_informacion(const crow::request &req)
{
    try
    {
        // auto &session = app.get_context<Session>(req);
        // if (auto status = Global::User::validPermissions(req, session, Global::User::allRoles);
        //     status.first != crow::status::OK)
        // {
        //     return crow::response(status.first);
        // }
        // else
        // {

        auto bodyParams = crow::json::load(req.body);

        auto db = std::make_unique<Configuracion>();

        const std::string a[5] =
        {
            bodyParams["razon_social"].s(),
            bodyParams["direccion"].s(),
            bodyParams["rfc"].s(),
            bodyParams["contacto"].s(),
            bodyParams["agradecimiento"].s()
        };

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

crow::response CConfiguracion::get_informacion(const crow::request &req)
{
    // auto &session = app.get_context<Session>(req);
    // if (auto status = Global::User::validPermissions(req, session,
    //                                                  {Global::User::Rol::Configuracion});
    //     status.first != crow::status::OK)
    // {
    //     return crow::response(status.first);
    // }
    // else
    auto db = std::make_unique<Configuracion>();
    auto list = db->get_conf_data(10, 14);
    {
        crow::json::wvalue response_json;
        response_json["razonSocial"] = list->get_item(0)->m_valor;
        response_json["direccion"] = list->get_item(1)->m_valor;
        response_json["rfc"] = list->get_item(2)->m_valor;
        response_json["contacto"] = list->get_item(3)->m_valor;
        response_json["agradecimiento"] = list->get_item(4)->m_valor;
        return crow::response(response_json);
    }
}

crow::response CConfiguracion::test_impresion(const crow::request &req)
{
    // auto &session = app.get_context<Session>(req);

    // if (auto status = Global::User::validPermissions(req, session, {Global::User::Rol::Configuracion});
    //     status.first != crow::status::OK)
    // {
    //     return crow::response(status.first);
    // }
    // else
    {
        std::string command = "echo \"" + Global::System::imprime_ticket(MLog::create(0,
                                                                                      0, 
                                                                                      "Test Impresion", 
                                                                                      100, 
                                                                                      0, 
                                                                                      100, 
                                                                                      "Completado", 
                                                                                      Glib::DateTime::create_now_local()), 0) + "\" | lp";
        std::system(command.c_str());

        Global::System::showNotify("Impresión", "Se imprimio hoja de prueba.", "dialog-information");

        return crow::response(crow::status::OK);
    }
}

crow::response CConfiguracion::reiniciar(const crow::request &req)
{
    // auto &session = app.get_context<Session>(req);
    // if (auto status = Global::User::validPermissions(req, session, {Global::User::Rol::Apagar_Equipo});
    //     status.first != crow::status::OK)
    // {
    //     return crow::response(status.first);
    // }
    // else
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
    // auto &session = app.get_context<Session>(req);
    // if (auto status = Global::User::validPermissions(req, session, {Global::User::Rol::Apagar_Equipo});
    //     status.first != crow::status::OK)
    // {
    //     return crow::response(status.first);
    // }
    // else
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
    // auto &session = app.get_context<Session>(req);
    // if (auto status = Global::User::validPermissions(req, session, {Global::User::Rol::Configuracion});
    //     status.first != crow::status::OK)
    // {
    //     return crow::response(status.first);
    // }
    // else
    {
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
}

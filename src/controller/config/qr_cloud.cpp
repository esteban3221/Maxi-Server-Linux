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
    Cloud::qr_url.clear();
    auto &database = Database::getInstance();
    auto config_url = database.sqlite3->command("SELECT valor FROM configuracion WHERE id = 101");

    if (!config_url || config_url->at("valor").empty())
        return;
    std::string server_url = config_url->at("valor")[0];

    std::thread([this, server_url]()
                {
                    auto future_status = std::async(
                        std::launch::async,
                        iniciar_vinculacion_qr,
                        server_url);


                    while (Cloud::qr_url.empty() && Cloud::poll_status_pair.load())
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));

                    if (!Cloud::qr_url.empty())
                        Glib::signal_idle().connect_once([this]()
                                                         { actualizar_qr(Cloud::qr_url); });

                    bool is_success = future_status.get();

                    Glib::signal_idle().connect_once([this, is_success]()
                                                     {
            auto mensaje = is_success ? "¡Vinculación Exitosa!" : "No es posible vincular en este momento";
            Global::System::showNotify("Maxicajero", mensaje, is_success ? "dialog-information" : "dialog-error"); }); })
        .detach();
}
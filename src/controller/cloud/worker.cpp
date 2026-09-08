#include "controller/cloud/worker.hpp"

void CloudWorker::process_queue()
{
    while (m_running)
    {
        try
        {
            sincronizar_logs_pendientes();
            checkAndApplyUpdate();
        }
        catch (const std::exception &e)
        {
            g_warning("Error en el worker de sincronización: %s", e.what());
        }

        std::this_thread::sleep_for(std::chrono::minutes(15));
    }
}

void CloudWorker::sincronizar_logs_pendientes()
{
    auto &database = Database::getInstance();
    auto result = database.sqlite3->command("SELECT Id, IdUser, Tipo, Descripcion, Ingreso, Cambio, Total, Estatus, Fecha, uuid_cloud FROM log WHERE sync_status = 0");

    if (!result || result->empty())
    {
        return;
    }

    size_t total_rows = result->at("Id").size();
    for (size_t i = 0; i < total_rows; ++i)
    {
        if (!m_running)
            break;

        auto mlog = MLog::create(
            std::stoull(result->at("Id")[i]),
            std::stoi(result->at("IdUser")[i]),
            result->at("Tipo")[i],
            result->at("Descripcion")[i],
            std::stod(result->at("Ingreso")[i]),
            std::stod(result->at("Cambio")[i]),
            std::stod(result->at("Total")[i]),
            result->at("Estatus")[i],
            Glib::DateTime::create_from_iso8601(result->at("Fecha")[i]),
            result->at("uuid_cloud")[i]);

        Cloud::sincronizar_con_nube_async(mlog);
    }
}

void CloudWorker::start()
{
    if (!m_running)
    {
        m_running = true;
        m_worker_thread = std::thread(&CloudWorker::process_queue, this);
        g_message("Worker de sincronización en la nube iniciado.");
    }
}

void CloudWorker::stop()
{
    if (m_running)
    {
        m_running = false;
        if (m_worker_thread.joinable())
        {
            m_worker_thread.join();
        }
        g_message("Worker de sincronización detenido.");
    }
}
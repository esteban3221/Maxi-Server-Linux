#pragma once
#include "model/log.hpp"
#include <atomic>

class Cloud
{
public:
    Cloud();
    ~Cloud();

    static crow::json::wvalue json_cloud(Glib::RefPtr<MLog> t_log, const std::string &device_uuid);
    const std::shared_ptr<ResultMap> get_logs_pendientes_sincronizar(int limite = 20);
    static void marcar_como_sincronizado(size_t id_log, const std::string &uuid_cloud);
    static void sincronizar_con_nube_async(Glib::RefPtr<MLog> t_log);

    static inline std::string qr_url;
    static inline std::atomic_bool poll_status_pair;
    static inline std::string pairing_code;
    static inline std::mutex cloud_mutex;

private:
    void sendData(const std::string &data);
    std::string receiveData();
};

bool iniciar_vinculacion_qr(const std::string &server_url);
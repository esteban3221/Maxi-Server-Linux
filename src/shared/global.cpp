#include "global.hpp"

namespace Global
{
    namespace Rest
    {
        crow::SimpleApp app;
        std::future<void> future;
    } // namespace Rest

    namespace EValidador
    {
        std::atomic<bool> is_running;
        std::atomic<bool> is_busy;

        std::atomic<bool> is_retry_connected;
        std::atomic<bool> is_connected;
        std::atomic<bool> is_wrong_port;
        std::atomic<bool> is_driver_correct;

        Balance balance;
    } // namespace EstadoValidador

    namespace Widget
    {
        Gtk::Stack *v_main_stack = nullptr;

        namespace Refill
        {
            Gtk::ColumnView *v_tree_reciclador_monedas = nullptr, *v_tree_reciclador_billetes = nullptr;
        } // namespace Refill

    } // namespace Widget

    namespace Utility
    {
        crow::json::wvalue obten_cambio(int &cambio, std::map<int, int> &reciclador)
        {
            std::vector<int> billsToReturn(reciclador.size(), 0); // Vector para almacenar la cantidad de billetes a devolver

            int index = reciclador.size() - 1; // Índice para insertar en el vector
            for (auto it = reciclador.rbegin(); it != reciclador.rend(); ++it, --index)
            {
                int denominacion = it->first / 100;         // Denominación del billete
                int &cantidadDisponible = it->second; // Cantidad disponible en el reciclador

                while (cambio >= denominacion && cantidadDisponible > 0)
                {
                    cambio -= denominacion; // Reducir el cambio
                    cantidadDisponible--;   // Usar un billete del reciclador
                    billsToReturn[index]++; // Incrementar el contador de billetes devueltos
                }
            }

            // Convertimos el vector a JSON
            crow::json::wvalue response;
            response = billsToReturn;

            return response; // Retornamos el JSON con el array
        }
        int find_position(const std::unordered_map<int, int>& index_map, int value) 
        {
            auto it = index_map.find(value);
            if (it != index_map.end()) {
                return it->second; // Retorna la posición
            }
            return -1; // Valor no encontrado
        }
    } // namespace Utility

    namespace ApiConsume
    {
        std::string token;

        const std::string URI = "http://localhost:5000";
        const std::string BASE_URL = URI + "/api/CashDevice";

        void autentica()
        {
            crow::json::wvalue json;

            json["Username"] = "admin";
            json["Password"] = "password";

            auto body = json.dump();

            auto r = cpr::Post(cpr::Url{URI + "/api/Users/Authenticate"},
                               cpr::Header{{"Content-Type", "application/json"},
                                           {"Authorization", "Bearer " + token}},
                               cpr::Body{body});

            if (r.status_code == crow::status::OK)
            {
                auto rcb = crow::json::load(r.text);
                token = rcb["token"].s();
            }
            else
                throw std::runtime_error("Controlador API REST no iniciado o error en el Servidor");
        }

    } // namespace ApiConsume

    namespace System
    {
        std::string exec(const char *cmd)
        {
            std::array<char, 128> buffer;
            std::string result;

            auto file_deleter = [](FILE *file)
            {
                if (file)
                    pclose(file);
            };

            std::unique_ptr<FILE, decltype(file_deleter)> pipe(popen(cmd, "r"), file_deleter);

            if (!pipe)
                throw std::runtime_error("popen() failed!");

            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
                result += buffer.data();

            result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
            return result;
        }

        std::string formatTime(int seconds)
        {
            int minutes = seconds / 60;
            int remainingSeconds = seconds % 60;

            std::ostringstream oss;
            oss << std::setw(2) << std::setfill('0') << minutes << ":"
                << std::setw(2) << std::setfill('0') << remainingSeconds;
            return oss.str();
        }

        /// @brief
        /// @param title
        /// @param subtitle
        /// @param type dialog-information
        void showNotify(const char *title, const char *subtitle, const char *type)
        {
            notify_init("MaxiCajero");
            NotifyNotification *Notify = notify_notification_new(title, subtitle, type);
            notify_notification_show(Notify, NULL);
            g_object_unref(G_OBJECT(Notify));
            notify_uninit();
        }
    } // namespace System

    namespace User
    {

    } // namespace User

} // namespace Helper

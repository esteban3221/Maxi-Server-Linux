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
        Gtk::Window *v_main_window = nullptr;        

        namespace Refill
        {
            Gtk::ColumnView *v_tree_reciclador_monedas = nullptr, *v_tree_reciclador_billetes = nullptr;
        } // namespace Refill

        namespace Impresora
        {
            bool state_vizualizacion[6]{false};
            Gtk::Switch *v_switch_impresion = nullptr;
        } // namespace Impresora

    } // namespace Widget

    namespace Utility
    {

        int total_anterior(const std::map<int, int> &map)
        {
            int total = 0;
            for (auto &&i : map)
                total += i.first * i.second;

            return total;
        }

        crow::json::wvalue json_ticket(Glib::RefPtr<MLog> t_log)
        {
            auto user = std::make_unique<Usuarios>();
            crow::json::wvalue data;
            data["ticket"] = crow::json::wvalue::list();

            data["ticket"][0]["id"] = t_log->m_id;
            data["ticket"][0]["usuario"] = user->get_usuarios(t_log->m_id_user)->m_usuario;
            data["ticket"][0]["fecha"] = t_log->m_fecha.format_iso8601();
            data["ticket"][0]["tipo"] = t_log->m_tipo;
            data["ticket"][0]["total"] = t_log->m_total;
            data["ticket"][0]["cambio"] = t_log->m_cambio;
            data["ticket"][0]["ingreso"] = t_log->m_ingreso;
            data["ticket"][0]["estatus"] = t_log->m_estatus;

            return data;
        }

        void valida_autorizacion(const crow::request &req, User::Rol rol)
        {
            auto auth_header = req.get_header_value("Authorization");
            if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ")
                throw BadRequestError("Error en la peticion, no se recibieron datos / token");

            std::string mycreds = auth_header.substr(7);
            if (mycreds != ApiConsume::token)
                throw UnauthorizedError("token invalido");
            UsuariosRoles u_roles;
            auto roles = u_roles.get_usuario_roles_by_id(User::id);
            if(roles)
            {
                for (size_t i = 0; i < roles->get_n_items(); i++)
                {
                    auto list = roles->get_item(i);
                    if (list->m_id_rol == static_cast<guint>(rol))
                        return;
                }
                throw UnauthorizedError("No autorizado para realizar esta operacion");
            }
            else
                throw ServerError("Error al cargar roles");
        }

        bool valida_administrador(const crow::request &req)
        {
            auto auth_header = req.get_header_value("Authorization");
            if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ")
                throw BadRequestError("Error en la peticion, no se recibieron datos / token");

            std::string mycreds = auth_header.substr(7);
            if (mycreds != ApiConsume::token)
                throw UnauthorizedError("token invalido");
            size_t cont_admin = 0;
            UsuariosRoles u_roles;
            auto roles = u_roles.get_usuario_roles_by_id(User::id);

            return roles->get_n_items() >= 19;
        }

        crow::json::wvalue obten_cambio(int &cambio, std::map<int, int> &reciclador, bool is_cambio)
        {
            std::vector<int> billsToReturn(reciclador.size(), 0); // Vector para almacenar la cantidad de billetes a devolver
            auto cambio_original = cambio; // Guardamos el valor original del cambio 
            int index = reciclador.size() - 1; // Índice para insertar en el vector
            for (auto it = reciclador.rbegin(); it != reciclador.rend(); ++it, --index)
            {
                int denominacion = it->first;         // Denominación del billete
                int &cantidadDisponible = it->second; // Cantidad disponible en el reciclador

                while ((is_cambio ? denominacion < cambio_original && cambio > 0 : cambio >= denominacion) && cantidadDisponible > 0)
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

        int find_position(const std::unordered_map<int, int> &index_map, int value)
        {
            for (auto &&i : index_map)
                if (i.second == value)
                    return i.first;

            return -1;
        }

        bool is_ok = true;
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
                               cpr::Header{{"Content-Type", "application/json"}/*,
                                           {"Authorization", "Bearer " + token}*/},
                               cpr::Body{body});

            if (r.status_code == crow::status::OK)
            {
                auto rcb = crow::json::load(r.text);
                token = rcb["token"].s();
            }
            else
                throw std::runtime_error("Controlador API REST no iniciado o error en el Servidor : " + r.reason);
        }

    } // namespace ApiConsume

    namespace System
    {
        bool stob(const std::string &data)
        {
            return data == "1";
        }

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
        std::string Current = "";
        int id = -1;
    } // namespace User

} // namespace Helper

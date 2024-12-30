#include "global.hpp"

namespace Global
{
    namespace Rest
    {
        crow::SimpleApp app;
        std::future<void> future;
    } // namespace Rest

    namespace Api_consume
    {
        std::string token;
        const std::string URI = "http://localhost:5000";
        const std::string BASE_URL = URI + "/api/CashDevice";

        void autentica()
        {
            nlohmann::json json;

            json["Username"] = "admin";
            json["Password"] = "password";

            auto body = json.dump();

            auto r = cpr::Post(cpr::Url{URI + "/api/Users/Authenticate"},
                          cpr::Header{{"Content-Type", "application/json"},
                                      {"Authorization", "Bearer " + token}},
                          cpr::Body{body});

            if (r.status_code == 200)
            {
                auto rcb = nlohmann::json::parse(r.text);
                token = rcb["token"].get<std::string>();
            }
            else
                throw "Controlador API REST no iniciado o error en el mismo";
        }

    } // namespace Api_consume

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
        // crow::status validUser(const crow::request &req, Session::context &session, std::string &userName)
        // {
        //     auto auth_header = req.get_header_value("Authorization");
        //     if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ")
        //         return crow::status::NOT_FOUND;

        //     auto keys = session.keys();
        //     for (const auto &key : keys)
        //     {
        //         userName = key;
        //         if (auth_header.substr(7) == session.string(key))
        //             return crow::status::OK;
        //     }
        //     return crow::status::UNAUTHORIZED;
        // }

        // std::pair<crow::status, std::string> validPermissions(const crow::request &req, Session::context &session, const std::vector<Rol> &vecRol)
        // {
        //     std::string userName;

        //     if (auto status = validUser(req, session, userName); status != crow::status::OK)
        //         return {status, userName};
        //     else
        //     {
        //         Model::usuarios_roles rol;
        //         auto rolesPorUsuario = rol.obten_roles_by_usuarios(userName);
        //         std::unordered_set<Rol> rolesNecesarios(vecRol.begin(), vecRol.end());
        //         for (const auto &rol_ : rolesPorUsuario["id_rol"])
        //         {
        //             Helper::User::Rol rolUsuarioEnum = static_cast<Helper::User::Rol>(std::stoi(rol_));

        //             if (rolesNecesarios.find(rolUsuarioEnum) != rolesNecesarios.end())
        //             {
        //                 rolesNecesarios.erase(rolUsuarioEnum); // Remueve el rol encontrado de rolesNecesarios
        //             }
        //         }
        //         if (rolesNecesarios.empty())
        //         {
        //             return {crow::status::OK, userName}; // Todos los roles necesarios están presentes, devuelve OK
        //         }
        //     }
        //     return {crow::status::UNAUTHORIZED, userName};
        // }

    } // namespace User

} // namespace Helper

#include "global.hpp"

namespace Global
{
    namespace Rest
    {
        crow::SimpleApp app;
        std::future<void> future;
    } // namespace Rest

    namespace Widget
    {
        Gtk::Stack *v_main_stack = nullptr;
        Gtk::Window *v_main_window = nullptr;        

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
    } // namespace Utility

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

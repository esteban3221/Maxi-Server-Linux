#pragma once
#include <crow.h>
#include <libnotify/notify.h>
#include <cpr/cpr.h>
#include <atomic>
#include <gtkmm.h>

#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */

namespace Global
{

    namespace Rest
    {
        // manejador de entradas a la api rest
        extern crow::SimpleApp app;

        // manejador asyncrono de app
        extern std::future<void> future;
    } // namespace Rest

    namespace Widget
    {
        // manejador global de vistas
        extern Gtk::Stack *v_main_stack;
        extern Gtk::Window *v_main_window;
        namespace Refill
        {
            extern Gtk::ColumnView *v_tree_reciclador_monedas, *v_tree_reciclador_billetes;
        } // namespace Refill
    } // namespace Widget

    namespace Utility
    {
        extern crow::json::wvalue obten_cambio(int &cambio, std::map<int, int> &reciclador);
        extern int total_anterior(const std::map<int, int> &map);
        extern int find_position(const std::unordered_map<int, int>& index_map, int value);
        extern void verifica_cambio(sigc::connection *conn, const std::chrono::steady_clock::time_point &start_time, const std::function<void()> &func);
    } // namespace Utility
    

    // por conveniencia algunas cosas estan en ingles
    namespace EValidador
    {
        extern std::atomic<bool> is_running;
        extern std::atomic<bool> is_busy;
        extern std::atomic<bool> is_connected;
        extern std::atomic<bool> is_retry_connected;
        extern std::atomic<bool> is_wrong_port;
        extern std::atomic<bool> is_driver_correct;

        struct Balance
        {
            std::atomic<uint32_t> ingreso = 0;
            std::atomic<uint32_t> ingreso_parcial = 0;
            std::atomic<uint32_t> total = 0;
            std::atomic<int32_t> cambio = 0;
        } extern balance;

        struct Conf
        {
            std::string puerto = "/dev/ttyUSB";
            uint32_t ssp = 0;
            std::string dispositivo;
            std::string log_ruta;
            bool habilita_recolector;
            bool auto_acepta_billetes;
        };
    } // namespace EstadoValidador

    namespace ApiConsume
    {
        extern std::string token;
        extern const std::string URI;
        extern const std::string BASE_URL;

        extern void autentica();
    } // namespace ApiConsume

    namespace System
    {
        extern std::string exec(const char *cmd);
        extern std::string formatTime(int seconds);
        extern void showNotify(const char *title, const char *subtitle, const char *type);

        extern bool stob(const std::string &data);
    } // namespace System

    class Async
    {
    private:
        std::queue<std::function<void()>> dispatch_queue;
        std::mutex dispatch_queue_mutex;

    public:
        Async(/* args */) {};
        ~Async() {};

        Glib::Dispatcher dispatcher;
        void on_dispatcher_emit();
        void dispatch_to_gui(std::function<void()> func);
    };

    inline void Async::on_dispatcher_emit()
    {
        std::function<void()> func;
        {
            std::lock_guard<std::mutex> lock(dispatch_queue_mutex);
            if (!dispatch_queue.empty())
            {
                func = dispatch_queue.front();
                dispatch_queue.pop();
            }
        }
        if (func)
        {
            func();
        }
    }

    inline void Async::dispatch_to_gui(std::function<void()> func)
    {
        {
            std::lock_guard<std::mutex> lock(dispatch_queue_mutex);
            dispatch_queue.push(func);
        }
        dispatcher.emit();
    }

    namespace User
    {

    } // namespace User

} // namespace Helper

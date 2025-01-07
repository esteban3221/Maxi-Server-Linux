#pragma once
#include <crow.h>
#include <libnotify/notify.h>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>
#include <atomic>


#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

namespace Global
{
    namespace Rest
    {
        //manejador de entradas a la api rest
        extern crow::SimpleApp app;

        //manejador asyncrono de app
        extern std::future<void> future;
    } // namespace Rest

    //por conveniencia esta en ingles
    namespace EValidador
    {
        extern std::atomic<bool> is_running;
        extern std::atomic<bool> is_busy;

        struct Balance
        {
            std::atomic<uint32_t> ingreso = 0;
            std::atomic<uint32_t> total = 0;
            std::atomic<int32_t> cambio = 0;
        }extern balance;

        struct Conf
        {
            std::string puerto = "/dev/ttyUSB";
            uint32_t ssp = 0;
            std::string nombre;
        }extern bill, coin;
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
    } // namespace System

    namespace User
    {

    } // namespace User

} // namespace Helper

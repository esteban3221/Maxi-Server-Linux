#pragma once
#include <crow.h>
#include <libnotify/notify.h>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

namespace Global
{
    namespace Rest
    {
        extern crow::SimpleApp app;

        //manejador asyncrono de app
        extern std::future<void> future;
    } // namespace Rest
    

    namespace Api_consume
    {
        extern std::string token;
        extern const std::string URI;
        extern const std::string BASE_URL;

        extern void autentica();
    } // namespace Api_consume

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

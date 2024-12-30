#pragma once
#include <iostream>
#include <functional>

namespace Model
{
    struct Usuario_historial_t
    {
        size_t id;
        size_t original_id;
        std::string old_username;
        std::string old_password;
        std::string new_username;
        std::string new_password;
        std::string fecha_modificacion;
        std::string usuario_operacion;
        std::string tipo_cambio;

        Usuario_historial_t() = default;
        explicit Usuario_historial_t(const std::function<void(Usuario_historial_t&)>& initializer) { initializer(*this); };
    };
    

    class Usuario_historial
    {
    private:
        /* data */
    public:
        Usuario_historial(/* args */);
        ~Usuario_historial();
    };
    
} // namespace Model
#pragma once
#include <iostream>
#include <functional>

namespace Model
{
    struct Usuarios_t
    {
        size_t id;
        std::string id_usuario;
        std::string passsword;

        Usuarios_t() = default;
        explicit Usuarios_t(const std::function<void(Usuarios_t&)>& initializer) { initializer(*this); };
    };
} // namespace Model

class Usuarios
{
private:
    /* data */
public:
    Usuarios(/* args */);
    ~Usuarios();
};
#pragma once
#include <iostream>
#include <functional>

namespace Model
{
    struct Usuarios_roles_t
    {
        size_t id;
        size_t id_usuario;
        u_int16_t id_rol;

        Usuarios_roles_t() = default;
        explicit Usuarios_roles_t(const std::function<void(Usuarios_roles_t&)>& initializer) { initializer(*this); };
    };
} // namespace Model

class Usuarios_roles
{
private:
    /* data */
public:
    Usuarios_roles(/* args */);
    ~Usuarios_roles();
};
#pragma once
#include <iostream>
#include <vector>
#include <functional>

namespace Model
{
    struct Roles_t
    {
        int id;
        std::string rol;

        Roles_t() = default;
        explicit Roles_t(const std::function<void(Roles_t&)>& initializer) { initializer(*this); };
    };
} // namespace Model

class Roles
{
private:
    /* data */
public:
    Roles(/* args */);
    ~Roles();
};
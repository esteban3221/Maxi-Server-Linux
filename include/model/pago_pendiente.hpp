#pragma once
#include <iostream>
#include <functional>

namespace Model
{
    struct Pago_pendiente_t
{
    int id;
    int id_log;
    int remanente;
    std::string status;

    Pago_pendiente_t() = default;
    explicit Pago_pendiente_t(const std::function<void(Pago_pendiente_t&)>& initializer) { initializer(*this); };
};
} // namespace Model



class Pago_pendiente
{
private:
    /* data */
public:
    Pago_pendiente(/* args */);
    ~Pago_pendiente();
};
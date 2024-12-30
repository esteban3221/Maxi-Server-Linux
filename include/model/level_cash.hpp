#pragma once
#include <vector>
#include <functional>

namespace Model
{
    struct Level_cash_t
    {
        int denominacion;
        int cant_alm;
        int cant_recy;
        int nivel_inmo;

        Level_cash_t() = default;
        explicit Level_cash_t(const std::function<void(Level_cash_t&)>& initializer) { initializer(*this); };
    };
} // namespace Model

class Level_cash
{
private:
    /* data */
public:
    Level_cash(/* args */);
    ~Level_cash();

};
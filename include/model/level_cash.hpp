#pragma once
#include <vector>

struct level_cash_
{
    int denominacion;
    int cant_alm;
    int cant_recy;
    int nivel_inmo;
};

class Level_cash
{
private:
    /* data */
public:
    Level_cash(/* args */);
    ~Level_cash();

    std::vector<level_cash_> get_levels();
};
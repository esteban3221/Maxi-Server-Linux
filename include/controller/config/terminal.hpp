#pragma once
#include "model/terminales.hpp"
#include "global.hpp"

namespace RestApp = Global::Rest;
class Terminal
{
private:
    crow::response get_all(const crow::request &req);
    crow::response get_by_id(const std::string &id);
    crow::response nueva(const crow::request &req);
    crow::response editar(const crow::request &req);
    crow::response eliminar(const crow::request &req);

    crow::response predeterminar(const crow::request &req);
public:
    Terminal(/* args */);
    ~Terminal();
};

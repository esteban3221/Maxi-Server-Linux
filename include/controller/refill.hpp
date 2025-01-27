#pragma once
#include "view/refill.hpp"
#include "level_cash.hpp"
#include "c_sharp_validator.hpp"
#include <map>

namespace RestApp = Global::Rest;
class Refill final : public VRefill
{
protected:
    void on_show();
    void func_poll(const std::string &status, const crow::json::rvalue &data);

private:
    Global::Async async_gui;

    crow::response inicia(const crow::request &req);
    crow::response deten(const crow::request &req);

    void init_data(Gtk::ColumnView *vcolumn, const std::string &tabla);
    void calcula_total();

    int total, total_bill, total_coin, parcial_bill, parcial_coin;

    const std::unordered_map<int,int> map_bill = {{1,20},{2,50},{3,100},{4,200},{5,500},{6,1'000}};
    const std::unordered_map<int,int> map_coin = {{1,1},{2,2},{3,5},{4,10}};

    std::shared_ptr<Gtk::SingleSelection> single_bill_selection, single_coin_selection;

public:
    Refill(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Refill();
};

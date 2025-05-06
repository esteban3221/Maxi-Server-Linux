#pragma once
#include "view/refill.hpp"
#include "level_cash.hpp"
#include "c_sharp_validator.hpp"
#include "log.hpp"
#include "controller/config/impresora.hpp"
#include <map>

namespace RestApp = Global::Rest;
class Refill final : public VRefill
{
protected:
    void on_show_map();
    void func_poll(const std::string &status, const crow::json::rvalue &data);

private:
    Global::Async async_gui;

    crow::response inicia(const crow::request &req);
    crow::response get_dashboard(const crow::request &req);
    void deten();

    void init_data(Gtk::ColumnView *vcolumn, const std::string &tabla);
    void calcula_total(const std::shared_ptr<Gtk::SingleSelection> &select_bill, const std::shared_ptr<Gtk::SingleSelection> &select_coin);

    int total, total_bill, total_coin, parcial_bill, parcial_coin;
    uint64_t total_parcial_monedas[4]{0}, total_parcial_billetes[6]{0};

    const std::unordered_map<int, int> map_bill = {
        {0, 20},
        {1, 50},
        {2, 100},
        {3, 200},
        {4, 500},
        {5, 1'000}};
    const std::unordered_map<int, int> map_coin = {
        {0, 1},
        {1, 2},
        {2, 5},
        {3, 10}};

    std::shared_ptr<Gtk::SingleSelection> single_bill_selection, single_coin_selection;

public:
    Refill(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Refill();
};

#pragma once
#include <condition_variable>
#include <mutex>

#include "view/metodo_pago.hpp"
#include "view/cortinilla_carga.hpp"
#include "model/configuracion.hpp"
#include "model/log.hpp"
#include "dialmonto.hpp"
#include "global.hpp"

// Metodos
#include "efectivo.hpp"
#include "tarjeta.hpp"

class MetodoPago : public VMetodoPago
{
private:
    enum class Metodo
    {
        EFECTIVO,
        TARJETA,
        MIXTO,
        NINGUNO
    } metodo_seleccionado;
    enum class Predeterminado
    {
        EFECTIVO,
        TARJETA,
        MIXTO
    };
    std::string get_metodo_nombre(Metodo);
    std::condition_variable cv_finalizado;
    std::mutex mtx_espera;

    bool transaccion_terminada = false;
    Glib::RefPtr<MLog> m_log;
    Log log;
    bool is_view_ingreso, is_mixto;
    size_t total_original = 0;
    Efectivo *efectivo_controller;
    std::unique_ptr<Tarjeta> tarjeta_controller;
    DialMonto *dial_monto;
    ViewCarga *cortinilla_carga;

    void btn_efectivo_on_click();
    void btn_tarjeta_on_click();
    void btn_diferido_on_click();
    void btn_cancelar_on_click();
    void on_dial_monto_entered(u_int64_t monto);
    void on_show_map();
    void reset_log();

    Predeterminado obtener_metodo_predeterminado();
    void pagar_por_metodo(Metodo metodo, size_t remanente = 0);

    crow::response procesa_pago(const crow::request &req);

public:
    MetodoPago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder, crow::SimpleApp &app);
    ~MetodoPago();
};
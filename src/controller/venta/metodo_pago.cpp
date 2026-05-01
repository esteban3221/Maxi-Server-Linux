#include "controller/venta/metodo_pago.hpp"

MetodoPago::MetodoPago(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder, crow::SimpleApp& app) : VMetodoPago(cobject, refBuilder)
{
    auto builder = Gtk::Builder::create_from_string(View::ui_vp);
    efectivo_controller = Gtk::Builder::get_widget_derived<Efectivo>(builder, "box", app);
    Global::Widget::v_main_stack->add(*efectivo_controller, "8", "Efectivo");

    //builder = Gtk::Builder::create_from_string(View::ui_vp);
    //tarjeta_controller = Gtk::Builder::get_widget_derived<Tarjeta>(builder, "box", app);
    //Global::Widget::v_main_stack->add(*tarjeta_controller, "9", "Tarjeta"); 

    builder = Gtk::Builder::create_from_string(View::ui_nip);
    dial_monto = Gtk::Builder::get_widget_derived<DialMonto>(builder, "box_nip");
    Global::Widget::v_main_stack->add(*dial_monto, "11", "DialMonto");

    dial_monto->signal_monto_entered.connect(sigc::mem_fun(*this, &MetodoPago::on_dial_monto_entered));
    v_btn_cash->signal_clicked().connect(sigc::mem_fun(*this, &MetodoPago::btn_efectivo_on_click));
    v_btn_card->signal_clicked().connect(sigc::mem_fun(*this, &MetodoPago::btn_tarjeta_on_click));
    v_btn_deferred->signal_clicked().connect(sigc::mem_fun(*this, &MetodoPago::btn_diferido_on_click));
    CROW_ROUTE(app, "/accion/inicia_venta").methods("POST"_method)(sigc::mem_fun(*this, &MetodoPago::procesa_pago));

}

MetodoPago::~MetodoPago()
{
}


crow::response MetodoPago::procesa_pago(const crow::request &req)
{   
    Sesion::valida_autorizacion(req, Global::User::Rol::Venta);
    auto param = crow::json::load(req.body);
    metodo_seleccionado = Metodo::NINGUNO;
    is_mixto = false;
    is_view_ingreso = param.has("is_view_ingreso") && param["is_view_ingreso"].b();

    m_log = MLog::create
    (
        0,
        Global::User::id,
        is_view_ingreso ? "Ingreso" : "Venta",
        param["concepto"].operator std::string(),
        0,
        0,
        param["value"].i(),
        "Creacion de evento",
        Glib::DateTime::create_now_local()
    );

    m_log->m_id = log.insert_log(m_log);
    total_original = m_log->m_total;

    switch (obtener_metodo_predeterminado())
    {
    case Predeterminado::EFECTIVO:
        pagar_por_metodo(Metodo::EFECTIVO);
        break;
    case Predeterminado::TARJETA:
        pagar_por_metodo(Metodo::TARJETA);
        break;
    case Predeterminado::MIXTO:
        Global::Widget::v_main_stack->set_visible_child(*this);
    default:
        break;
    }

    return(Log::json_ticket(m_log));
}


MetodoPago::Predeterminado MetodoPago::obtener_metodo_predeterminado()
{
    /*id 24 = 0/1 permite diferir*/
    auto config = std::make_unique<Configuracion>();
    auto data_conf = config->get_conf_data(24, 25);
    auto metodo = std::stoi(data_conf->get_item(0)->m_valor);

    v_btn_deferred->set_visible(data_conf->get_item(1)->m_valor == "1");
    if (metodo_seleccionado == Metodo::MIXTO)
        v_btn_deferred->set_visible(false);

    switch (metodo)
    {    
        case 0:
            return Predeterminado::EFECTIVO;
        case 1:
            return Predeterminado::TARJETA;
        case 2:
            return Predeterminado::MIXTO;
        default:
            return Predeterminado::EFECTIVO;
    }
}

void MetodoPago::pagar_por_metodo(Metodo metodo, size_t remanente)
{
    Glib::signal_idle().connect_once([this, metodo, remanente]()
    {
        if(remanente > 0)  
            m_log->m_total = remanente;

        dial_monto->property_monto_dial() = (m_log->m_total - m_log->m_ingreso);
        
        switch (metodo)
        {
            case Metodo::EFECTIVO:
                efectivo_controller->inicia(m_log, is_view_ingreso);
                break;
            case Metodo::TARJETA:
                // Ir a vista de tarjeta, pasar el log y el remanente
                break;
            default:
                break;
        }

        if(m_log->m_ingreso >= total_original)
            Global::Widget::v_main_stack->set_visible_child(Global::Widget::default_home);
        else
            Global::Widget::v_main_stack->set_visible_child(*this);
    });
}

std::string MetodoPago::get_metodo_nombre(Metodo m) {
    switch (m) {
        case Metodo::EFECTIVO: return "EFECTIVO";
        case Metodo::TARJETA:  return "TARJETA";
        case Metodo::MIXTO:    return "MIXTO";
        default:               return "NO_SELECCIONADO";
    }
}

void MetodoPago::btn_efectivo_on_click()
{
    metodo_seleccionado = Metodo::EFECTIVO;
    if (is_mixto)
        Global::Widget::v_main_stack->set_visible_child("11");
    else
        pagar_por_metodo(metodo_seleccionado);
}
void MetodoPago::btn_tarjeta_on_click()
{
    metodo_seleccionado = Metodo::TARJETA;
    if (is_mixto)
        Global::Widget::v_main_stack->set_visible_child("11");
    else
        pagar_por_metodo(metodo_seleccionado);
}

void MetodoPago::btn_diferido_on_click()
{
    metodo_seleccionado = Metodo::MIXTO;
    is_mixto = true;
    btn_tarjeta_on_click();
}


void MetodoPago::on_dial_monto_entered(u_int64_t monto)
{

    if(monto > total_original - m_log->m_ingreso)
    {
        Global::System::showNotify("Error", "El monto ingresado excede el total restante por pagar.", "dialog-error");
        return;
    }

    // Si el monto es menor/igual al total, se asume que se esta pagando una parte del total con el metodo seleccionado
    pagar_por_metodo(metodo_seleccionado, monto);
}
#include "controller/config/empresa.hpp"
#include "empresa.hpp"

Empresa::Empresa(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VEmpresa(cobject, refBuilder)
{
    signal_map().connect(sigc::mem_fun(*this, &Empresa::init_data));

    for (size_t i = 0; i < 5; i++)
        v_list_ety_datos[i]->signal_changed().connect(sigc::mem_fun(*this, &Empresa::actualiza_datos));
}

Empresa::~Empresa()
{
}

void Empresa::init_data()
{
    auto db = std::make_unique<Configuracion>();
    auto db_empresa = db->get_conf_data(10, 14);

    for (size_t i = 0; i < db_empresa->get_n_items(); i++)
        v_list_ety_datos[i]->set_text(db_empresa->get_item(i)->m_valor);
}

void Empresa::actualiza_datos()
{
    auto db = std::make_unique<Configuracion>();

    for (size_t i = 0; i < 5; i++)
        db->update_conf(MConfiguracion::create(i + 10, std::to_string(i) + " - Datos Empresa",v_list_ety_datos[i]->get_text())); 
}

#include "controller/config/general.hpp"
#include "general.hpp"

General::General(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : VGeneral(cobject, refBuilder) 
{
    //@@@ por el momento no se tiene una vista de carrousel
    v_Drop_temporizador->set_sensitive(false);
    v_btn_select_carrousel->set_sensitive(false);

    carga_estado_inicial();
}

General::~General()
{
}

void General::carga_estado_inicial()
{
    auto db = std::make_unique<Configuracion>();
    auto db_general = db->get_conf_data(5,9);

    v_check_config_notifi->set_active(db_general->get_item(0)->m_valor == "1");
    v_lbl_path_icon->set_text(db_general->get_item(1)->m_valor);
    v_ety_mensaje_inicio->set_text(db_general->get_item(2)->m_valor);
    v_lbl_path_carrousel->set_text(db_general->get_item(3)->m_valor);
    v_Drop_temporizador->set_selected(std::stoi(db_general->get_item(4)->m_valor));

    db_general = db->get_conf_data(10,14);
}
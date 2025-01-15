#include "view/config/base.validador.hpp"
#include "base.validador.hpp"

VValidador::VValidador(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder) : m_builder{refBuilder},
                                                                                                Gtk::Box(cobject),
                                                                                                id_conf_(-1)
{
    for (u_int16_t i = 0; i < 11; i++)
        v_lbl_data[i] = m_builder->get_widget<Gtk::Label>("lbl_data_" + std::to_string(i));

    v_lbl_titulo = m_builder->get_widget<Gtk::Label>("lbl_titulo");
    v_drop_puerto = m_builder->get_widget<Gtk::DropDown>("drop_puerto");

    // señales
    this->signal_map().connect(sigc::mem_fun(*this, &VValidador::on_show));
}

void VValidador::llena_puertos()
{
    std::string path = "/dev";
    puertos.clear();
    try
    {
        for (const auto &entry : std::filesystem::directory_iterator(path))
            if (entry.is_character_file() && entry.path().string().find("ttyUSB") != std::string::npos)
                puertos.emplace_back(entry.path().string());
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

VValidador::~VValidador()
{
}

void VValidador::set_id_conf(uint16_t id)
{
    id_conf_ = id;
}

void VValidador::set_data_lbl(const std::string &json)
{
    auto j = crow::json::load(json);
    set_data_lbl(j);
}

void VValidador::set_data_lbl(const crow::json::rvalue &json)
{
    std::string titulo{json["deviceModel"].s()};

    std::string id{json["deviceID"].s()};
    std::string conectado{json["isOpen"].b() ? "Si" : "No"};
    std::string ssp_ver{std::to_string(json["sspProtocolVersion"].i())};
    std::string error{json["deviceError"].s()};
    std::string firmware{json["firmware"].s()};
    std::string set_regional{json["dataset"].s()};

    std::string serial{json.has("validatorSerialNumber") ? json["validatorSerialNumber"].s() : json["primaryHopperSerialNumber"].s()};
    std::string modulo{json.has("payoutModuleSerialNumber") ? json["payoutModuleSerialNumber"].s() : json["coinFeederSerialNumber"].s()};
    std::string revision{json.has("validatorRevision") ? json["validatorRevision"].s() : json["primaryHopperBuildRevision"].s()};
    std::string modulo_revision{json.has("payoutModuleRevision") ? json["payoutModuleRevision"].s() : json["coinFeederBuildRevision"].s()};

    std::string reloj{json["realTimeClock"].s()};

    v_lbl_titulo->set_text(titulo);

    v_lbl_data[0]->set_text(id);
    v_lbl_data[1]->set_text(conectado);
    v_lbl_data[2]->set_text(ssp_ver);
    v_lbl_data[3]->set_text(error);
    v_lbl_data[4]->set_text(firmware);
    v_lbl_data[5]->set_text(set_regional);

    v_lbl_data[6]->set_text(serial);
    v_lbl_data[7]->set_text(modulo);
    v_lbl_data[8]->set_text(revision);
    v_lbl_data[9]->set_text(modulo_revision);

    v_lbl_data[10]->set_text(reloj);
}

void VValidador::on_show()
{
    llena_puertos();

    if (m_list_puertos)
        while (m_list_puertos->get_n_items() > 0)
            m_list_puertos->remove(0);

    m_list_puertos = Gtk::StringList::create(puertos);
    v_drop_puerto->set_model(m_list_puertos);

    auto db_conf = std::make_unique<Configuracion>();
    auto data_puerto = db_conf->get_conf_data(id_conf_, id_conf_);
    auto puerto_db = data_puerto->get_item(0)->m_valor;

    for (size_t i = 0; i < m_list_puertos->get_n_items(); i++)
    {
        v_drop_puerto->set_selected(i);

        if (puertos[i] == puerto_db)
            break;
    }
}

Glib::ustring VValidador::get_puerto_seleccionado()
{
    const auto selected = v_drop_puerto->get_selected();
    return m_list_puertos->get_string(selected);
}

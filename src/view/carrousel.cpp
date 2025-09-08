#include "carrousel.hpp"

VCarrousel::VCarrousel(/* args */)
{
    set_transition_type(Gtk::StackTransitionType::ROTATE_LEFT_RIGHT);
    set_transition_duration(1000);
    obtener_tiempo_carrousel();
    init();
}

void VCarrousel::init(void)
{
    std::vector<Gtk::Image *> vec_imgs;
    using namespace Global::Widget;
    auto db = std::make_unique<Configuracion>();
    auto db_general = db->get_conf_data(8, 9);
    db_general->get_item(1)->m_valor == "0" ? default_home = "0" : default_home = "10";

    for (auto &&i : listar_contenido(db_general->get_item(0)->m_valor))
    {
        auto img = Gtk::manage(new Gtk::Image());
        img->set(i);
        vec_imgs.push_back(img);
    }
    set_image_pages(vec_imgs);
    init_imgs();
}

void VCarrousel::set_image_pages(const std::vector<Gtk::Image*> &vec_images)
{
    clear_pages();
    for (auto* image : vec_images)
    {
        auto new_image = std::make_unique<Gtk::Image>();
            new_image->set(image->get_paintable());
        
        vec_pages.push_back(std::move(new_image));
    }
}

std::vector<std::string> VCarrousel::listar_contenido(const std::string &ruta_carpeta)
{
    std::vector<std::string> archivos;
    try
    {
        for (const auto & entry : std::filesystem::directory_iterator(ruta_carpeta))
        {
            if(entry.is_regular_file())
                archivos.push_back(entry.path().string());
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        std::cerr << "Error al acceder al directorio: " << e.what() << std::endl;
    }
    return archivos;
}

bool VCarrousel::mueve_carrousel()
{
    for (auto &&i : vec_pages)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(mili_seconds_move));
        set_visible_child(*i);
    }

    return not vec_pages.empty();
}

void VCarrousel::obtener_tiempo_carrousel()
{
    size_t millis = 0;
    using namespace Global::Widget;
    auto db = std::make_unique<Configuracion>();
    auto db_general = db->get_conf_data(9, 9);
    switch (static_cast<DropdownTime>(std::stoi(db_general->get_item(0)->m_valor)))
    {
        
        case DropdownTime::NONE: default_home = "0"; break;
        case DropdownTime::FIVE: millis = 5000; default_home = "10"; break;
        case DropdownTime::TEN: millis = 10000; default_home = "10"; break;
        case DropdownTime::FIFTEEN: millis = 15000; default_home = "10"; break;
        case DropdownTime::THIRTY: millis = 30000; default_home = "10"; break;
        case DropdownTime::ONE_MIN: millis = 60000; default_home = "10"; break;
        default: default_home = "10"; break;
    }

    mili_seconds_move = millis;
}

void VCarrousel::clear_pages()
{
    if(conn.connected())
        conn.disconnect();
    for (auto &&i : vec_pages)
        remove(*i);  
    vec_pages.clear();
}

VCarrousel::~VCarrousel()
{
    clear_pages();
}

void VCarrousel::init_imgs(void)
{
    if (vec_pages.size() > 0)
    {
        for (auto &&i : vec_pages)
            add(*i);
        
        // conn = Glib::signal_timeout().connect(sigc::mem_fun(*this, &VCarrousel::mueve_carrousel), vec_pages.size() * mili_seconds_move);
        std::thread([this]()
        {
            while (mili_seconds_move != 0)
            {
                mueve_carrousel();
                std::this_thread::sleep_for(std::chrono::seconds(vec_pages.size() * mili_seconds_move / 1000));
            }
        }).detach();
    }
    else
        std::cout << "No se encontro ningun elemento a mostrar\n";
}

namespace Global
{
    namespace Widget
    {
        VCarrousel *v_carrousel = nullptr;
        std::string default_home;
    }
}
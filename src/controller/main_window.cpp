#include "controller/main_window.hpp"
#include "main_window.hpp"

MainWindow::MainWindow(/* args */) : contador_click(0),
                                        sesion(app),
                                        log_data(app),
                                        configuracion(app),
                                        terminales(app)
{
    v_btn_pill->set_opacity(1);

    v_box_principal->signal_map().connect(sigc::mem_fun(*this, &MainWindow::on_map_view));
    v_btn_logo_nip->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_btn_pill_clicked));


    // widgets dervados
    auto builder = Gtk::Builder::create_from_string(View::ui_vp);
    auto pago = Gtk::Builder::get_widget_derived<Pago>(builder, "box", app);
    Global::Widget::v_main_stack->add(*pago, "1", "Pago");

    builder = Gtk::Builder::create_from_string(View::ui_vp);
    auto venta = Gtk::Builder::get_widget_derived<Venta>(builder, "box", app);
    Global::Widget::v_main_stack->add(*venta, "3", "Venta");

    builder = Gtk::Builder::create_from_string(View::ui_vp);
    auto pago_manual = Gtk::Builder::get_widget_derived<PagoManual>(builder, "box");
    Global::Widget::v_main_stack->add(*pago_manual, "2", "Pago Manual");

    builder = Gtk::Builder::create_from_string(View::ui_refill);
    auto refill = Gtk::Builder::get_widget_derived<Refill>(builder, "boxVistaEfectivo", app);
    Global::Widget::v_main_stack->add(*refill, "4", "Refill");

    Global::Widget::v_main_stack->add(*Gtk::manage(new Config), "5", "Configuracion");

    builder = Gtk::Builder::create_from_string(View::ui_nip);
    auto nip = Gtk::Builder::get_widget_derived<Nip>(builder, "box_nip");
    Global::Widget::v_main_stack->add(*nip, "6", "Nip");

    builder = Gtk::Builder::create_from_string(View::ui_nip);
    auto nnip = Gtk::Builder::get_widget_derived<NuevoNip>(builder, "box_nip");
    Global::Widget::v_main_stack->add(*nnip, "7", "Nuevo Nip");

    Global::Widget::v_carrousel = Gtk::manage(new VCarrousel);
    Global::Widget::v_main_stack->add(*Global::Widget::v_carrousel, "10", "Carrousel");

    Global::Widget::v_main_stack->set_transition_type(Gtk::StackTransitionType::SLIDE_LEFT_RIGHT);

    // inicializacion de servidor
    future = app.port(44333).loglevel(crow::LogLevel::Info).multithreaded().run_async();
}

void MainWindow::on_btn_pill_clicked()
{
    if (contador_click >= 4)
    {
        Global::Widget::v_main_stack->set_visible_child("6");
        contador_click = 0;
    }
    else
    {
        contador_click++;
        v_btn_pill->set_label(std::to_string(contador_click));
    }
}

void MainWindow::on_map_view()
{
    auto db_conf = std::make_unique<Configuracion>();
    auto data = db_conf->get_conf_data(6,7);

    v_img_main_logo->property_file() = data->get_item(0)->m_valor;
    v_lbl_main->set_text(data->get_item(1)->m_valor);
    
}

MainWindow::~MainWindow()
{
    app.stop();
    future.get();
}
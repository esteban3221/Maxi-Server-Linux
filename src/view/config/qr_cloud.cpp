#include "view/config/qr_cloud.hpp"

VQrCloud::VQrCloud(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder)
    : Gtk::Box(cobject), m_builder(refBuilder)
{
    this->v_picture_qr = m_builder->get_widget<Gtk::Picture>("v_picture");
    this->v_label_uuid = m_builder->get_widget<Gtk::Label>("v_label_uuid");
    this->v_label_pin = m_builder->get_widget<Gtk::Label>("v_label_pin");
    this->v_label_status = m_builder->get_widget<Gtk::Label>("v_label_status");
    this->v_dropdown_canal = m_builder->get_widget<Gtk::DropDown>("v_dropdown_canal");
}

VQrCloud::~VQrCloud()
{
}

void VQrCloud::draw_rounded_rect(const Cairo::RefPtr<Cairo::Context> &cr, double x, double y, double width, double height, double radius)
{
    cr->begin_new_sub_path();
    cr->arc(x + width - radius, y + radius, radius, -M_PI / 2, 0);
    cr->arc(x + width - radius, y + height - radius, radius, 0, M_PI / 2);
    cr->arc(x + radius, y + height - radius, radius, M_PI / 2, M_PI);
    cr->arc(x + radius, y + radius, radius, M_PI, 3 * M_PI / 2);
    cr->close_path();
}

void VQrCloud::actualizar_qr(const std::string &url)
{
    if (url.empty())
        return;

    auto texture = crear_qr_estilizado(url, 15);
    if (texture)
        v_picture_qr->set_paintable(texture);
}

Glib::RefPtr<Gdk::Texture> VQrCloud::crear_qr_estilizado(const std::string &texto, int module_size)
{
    QRcode *qr = nullptr;
    try
    {
        qr = QRcode_encodeString(texto.c_str(), 0, QR_ECLEVEL_H, QR_MODE_8, 1);

        if (!qr)
        {
            g_warning("Fallo al generar el código QR con libqrencode.");
            return nullptr;
        }

        int qr_size = qr->width;
        int border = 2;
        int img_size = (qr_size + border * 2) * module_size;

        auto surface = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, img_size, img_size);
        auto cr = Cairo::Context::create(surface);

        cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
        draw_rounded_rect(cr, 0, 0, img_size, img_size, 20.0);
        cr->fill();

        cr->set_source_rgba(0.1, 0.14, 0.49, 1.0);

        for (int y = 0; y < qr_size; ++y)
        {
            for (int x = 0; x < qr_size; ++x)
            {
                bool is_dark = (qr->data[y * qr_size + x] & 1) == 1;

                if (is_dark)
                {
                    double px = (x + border) * module_size;
                    double py = (y + border) * module_size;

                    bool is_finder = (x < 7 && y < 7) ||            // Esquina superior izquierda
                                     (x >= qr_size - 7 && y < 7) || // Esquina superior derecha
                                     (x < 7 && y >= qr_size - 7);   // Esquina inferior izquierda

                    if (is_finder)
                    {
                        cr->rectangle(px, py, module_size, module_size);
                        cr->fill();
                    }
                    else
                    {
                        double radius = (module_size / 2.0) * 0.85;
                        cr->arc(px + module_size / 2.0, py + module_size / 2.0, radius, 0, 2 * M_PI);
                        cr->fill();
                    }
                }
            }
        }

        surface->flush();
        int width = surface->get_width();
        int height = surface->get_height();
        int stride = surface->get_stride();
        const unsigned char *data = surface->get_data();

        auto bytes = Glib::Bytes::create(data, height * stride);

        QRcode_free(qr);

        return Gdk::MemoryTexture::create(
            width, height,
            Gdk::MemoryTexture::Format::B8G8R8A8_PREMULTIPLIED,
            bytes,
            stride);
    }
    catch (const std::exception &e)
    {
        if (qr)
        {
            QRcode_free(qr);
        }
        g_warning("Error al generar el QR: %s", e.what());
        return nullptr;
    }
}

namespace View
{
    const char *ui_qr_cloud = R"(<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <requires lib="gtk" version="4.0"/>
  <object class="GtkBox" id="view_qr_cloud">
    <property name="orientation">1</property>
    <property name="spacing">18</property>
    <property name="margin-start">24</property>
    <property name="margin-end">24</property>
    <property name="margin-top">24</property>
    <property name="margin-bottom">24</property>
    <child>
      <object class="GtkLabel" id="v_label_titulo">
        <property name="label">Maxi Cloud</property>
        <property name="halign">1</property>
        <style>
          <class name="title-1"/>
        </style>
      </object>
    </child>
    <child>
      <object class="GtkLabel" id="v_label_subtitulo">
        <property name="label">Para tener tu Maxi Server en la nube, escanea el siguiente código QR con tu celular y sigue los pasos. Si ya tenes tu cuenta de Maxi Cloud, inicia sesión y agrega tu Maxi Server a tu cuenta.</property>
        <property name="halign">1</property>
        <property name="wrap">true</property>
        <property name="max-width-chars">60</property>
        <style>
          <class name="dim-label"/>
        </style>
      </object>
    </child>
    <child>
      <object class="GtkBox">
        <property name="orientation">1</property>
        <property name="halign">3</property>
        <property name="vexpand">true</property>
        <property name="margin-top">12</property>
        <child>
          <object class="GtkScrolledWindow" id="v_scrolled_window">
            <property name="hscrollbar-policy">2</property>
            <property name="vscrollbar-policy">1</property>
            <property name="vexpand">true</property>
            <property name="child">
              <object class="GtkListBox" id="v_listbox">
                <property name="valign">3</property>
                <property name="halign">3</property>
                <style>
                  <class name="boxed-list"/>
                  <class name="rich-list"/>
                </style>
                <child>
                  <object class="GtkListBoxRow">
                    <property name="activatable">false</property>
                    <property name="child">
                      <object class="GtkPicture" id="v_picture">
                        <property name="width-request">200</property>
                        <property name="height-request">200</property>
                        <property name="margin-start">12</property>
                        <property name="margin-end">12</property>
                        <property name="margin-top">12</property>
                        <property name="margin-bottom">12</property>
                      </object>
                    </property>
                  </object>
                </child>
                <child>
                  <object class="GtkListBoxRow">
                    <property name="activatable">false</property>
                    <property name="child">
                      <object class="GtkBox">
                        <property name="margin-start">12</property>
                        <property name="margin-end">12</property>
                        <property name="margin-top">12</property>
                        <property name="margin-bottom">12</property>
                        <child>
                          <object class="GtkLabel">
                            <property name="label">UUID:</property>
                            <property name="halign">1</property>
                            <property name="valign">3</property>
                            <property name="xalign">0</property>
                          </object>
                        </child>
                        <child>
                          <object class="GtkLabel" id="v_label_uuid">
                            <property name="label">Cargando...</property>
                            <property name="hexpand">true</property>
                            <property name="halign">2</property>
                            <property name="valign">3</property>
                            <property name="xalign">1</property>
                            <style>
                              <class name="monospace"/>
                            </style>
                          </object>
                        </child>
                      </object>
                    </property>
                  </object>
                </child>
                <child>
                  <object class="GtkListBoxRow">
                    <property name="activatable">false</property>
                    <property name="child">
                      <object class="GtkBox">
                        <property name="margin-start">12</property>
                        <property name="margin-end">12</property>
                        <property name="margin-top">12</property>
                        <property name="margin-bottom">12</property>
                        <child>
                          <object class="GtkLabel">
                            <property name="label">Estado</property>
                            <property name="halign">1</property>
                            <property name="valign">3</property>
                            <property name="xalign">0</property>
                          </object>
                        </child>
                        <child>
                          <object class="GtkLabel" id="v_label_status">
                            <property name="label">Desconectado</property>
                            <property name="hexpand">true</property>
                            <property name="halign">2</property>
                            <property name="valign">3</property>
                            <property name="xalign">1</property>
                          </object>
                        </child>
                      </object>
                    </property>
                  </object>
                </child>
                <child>
                  <object class="GtkListBoxRow">
                    <property name="activatable">false</property>
                    <property name="child">
                      <object class="GtkBox">
                        <property name="margin-start">12</property>
                        <property name="margin-end">12</property>
                        <property name="margin-top">12</property>
                        <property name="margin-bottom">12</property>
                        <child>
                          <object class="GtkLabel">
                            <property name="label">PIN de vinculación</property>
                            <property name="halign">1</property>
                            <property name="valign">3</property>
                            <property name="xalign">0</property>
                          </object>
                        </child>
                        <child>
                          <object class="GtkLabel" id="v_label_pin">
                            <property name="label">----</property>
                            <property name="hexpand">true</property>
                            <property name="halign">2</property>
                            <property name="valign">3</property>
                            <property name="xalign">1</property>
                            <style>
                              <class name="heading"/>
                            </style>
                          </object>
                        </child>
                      </object>
                    </property>
                  </object>
                </child>
                <child>
                  <object class="GtkListBoxRow">
                    <property name="activatable">false</property>
                    <property name="child">
                      <object class="GtkBox">
                        <property name="margin-start">12</property>
                        <property name="margin-end">12</property>
                        <property name="margin-top">12</property>
                        <property name="margin-bottom">12</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkLabel">
                            <property name="label">Canal de actualizaciones</property>
                            <property name="halign">1</property>
                            <property name="valign">3</property>
                            <property name="hexpand">true</property>
                            <property name="xalign">0</property>
                          </object>
                        </child>
                        <child>
                          <object class="GtkDropDown" id="v_dropdown_canal">
                            <property name="valign">3</property>
                            <property name="halign">2</property>
                            <property name="model">
                              <object class="GtkStringList">
                                <items>
                                  <item>LTS (Estable)</item>
                                  <item>Test (Pruebas)</item>
                                </items>
                              </object>
                            </property>
                          </object>
                        </child>
                      </object>
                    </property>
                  </object>
                </child>
              </object>
            </property>
          </object>
        </child>
      </object>
    </child>
  </object>
</interface>
)";
}
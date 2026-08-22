#include "view/config/qr_cloud.hpp"

VQrCloud::VQrCloud()
{
    append(v_picture);
    v_picture.set_expand(true);
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

    auto texture = crear_qr_estilizado(url, 15); // 15 = tamaño en píxeles de cada puntito del QR
    if (texture)
    {
        v_picture.set_paintable(texture);
    }
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
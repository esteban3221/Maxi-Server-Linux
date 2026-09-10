#pragma once
#include "controller/cloud/cloud.h"
#include "view/config/qr_cloud.hpp"
#include "model/configuracion.hpp"
#include <future>

class QrCloud : public VQrCloud
{
public:
    QrCloud(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~QrCloud();

private:
    void on_show_map(void);
    void on_dropdown_canal_changed();
};

#pragma once
#include "controller/cloud/cloud.h"
#include "view/config/qr_cloud.hpp"
#include <future>

class QrCloud : public VQrCloud
{
public:
    QrCloud();
    ~QrCloud();

private:
    void on_show_map(void);
};

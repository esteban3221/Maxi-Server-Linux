#include "view/base.nip.hpp"
#include "global.hpp"
#include "model/configuracion.hpp"

class Nip : public VBaseNip
{
private:
    void on_btn_nip_enter() override;
    void on_btn_nip_back() override;
public:
    Nip(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~Nip();
};
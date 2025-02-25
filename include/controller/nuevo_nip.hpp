#include "view/base.nip.hpp"

class NuevoNip : public VBaseNip
{
private:
    void on_btn_nip_enter() override;
    void on_btn_nip_back() override;
public:
    NuevoNip(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~NuevoNip();
};
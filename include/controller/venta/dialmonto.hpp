#include "view/base.nip.hpp"
#include "global.hpp"
#include <sigc++/sigc++.h>

class DialMonto : public VBaseNip
{
protected:
    Gtk::Label v_lbl_monto;
private:
    void on_btn_nip_enter() override;
    void on_btn_nip_back() override;

    void on_map_show_map();
    u_int64_t monto; // monto pretendido a pagar
public:
    DialMonto(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder);
    ~DialMonto();

    u_int64_t &property_monto_dial() { return monto; }
    sigc::signal<void(u_int64_t)> signal_monto_entered;
};

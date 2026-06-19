#pragma once
#include <gtkmm.h>
#include <pangomm.h>
#include <vector>

// We derive our own class from PrintOperation,
// so we can put the actual print implementation here.
class PrintFormOperation : public Gtk::PrintOperation
{
public:
    static Glib::RefPtr<PrintFormOperation> create();
    virtual ~PrintFormOperation();

    void set_markup(const Glib::ustring &markup)
    {
        this->markup = markup;
    }

protected:
    PrintFormOperation();

    // PrintOperation default signal handler overrides:
    void on_begin_print(const Glib::RefPtr<Gtk::PrintContext> &context) override;
    void on_draw_page(const Glib::RefPtr<Gtk::PrintContext> &context, int page_nr) override;

    Glib::ustring markup;
    Glib::RefPtr<Pango::Layout> m_refLayout;
    std::vector<int> m_PageBreaks; // line numbers where a page break occurs
};
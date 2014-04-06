#ifndef ZEN_EWMH_HPP
#define ZEN_EWMH_HPP

#include "xpp/xpp.hpp"
#include "types.hpp"
#include "cursor.hpp"

namespace zen {

class ewmh
  : public xpp::event::sink<x::property_notify>
{
  public:
    // ewmh(x::connection & c,
    //       x::registry & registry,
    //       zen::cursor & cursor,
    //       uint8_t button)
    //   : m_c(c)
    //   , m_registry(registry)
    //   , m_cursor(cursor[XC_fleur])
    //   , m_button(button)
    // {}

    void handle(const x::property_notify & e)
    {
    }

    void
    get_net_supported(void)
    {
    }

    void
    set_net_supported(void)
    {
    }

    void
    get_net_client_list(void)
    {
    }

    void
    set_net_client_list(void)
    {
    }

    void
    get_net_number_of_desktops(void)
    {
    }

    void
    set_net_number_of_desktops(void)
    {
    }

    void
    get_net_desktop_geometry(void)
    {
    }

    void
    set_net_desktop_geometry(void)
    {
    }

    void
    get_net_desktop_viewport(void)
    {
    }

    void
    set_net_desktop_viewport(void)
    {
    }

    void
    get_net_current_desktop(void)
    {
    }

    void
    set_net_current_desktop(void)
    {
    }

    void
    get_net_desktop_names(void)
    {
    }

    void
    set_net_desktop_names(void)
    {
    }

    void
    get_net_active_window(void)
    {
    }

    void
    set_net_active_window(void)
    {
    }

    void
    get_net_workarea(void)
    {
    }

    void
    set_net_workarea(void)
    {
    }

    void
    get_net_supporting_wm_check(void)
    {
    }

    void
    set_net_supporting_wm_check(void)
    {
    }

    void
    get_net_virtual_roots(void)
    {
    }

    void
    set_net_virtual_roots(void)
    {
    }

    void
    get_net_desktop_layout(void)
    {
    }

    void
    set_net_desktop_layout(void)
    {
    }

    void
    get_net_showing_desktop(void)
    {
    }

    void
    set_net_showing_desktop(void)
    {
    }

    // other

    void
    handle_net_close_window(void)
    {
    }

    void
    handle_net_moveresize_window(void)
    {
    }

    void
    handle_net_wm_moveresize(void)
    {
    }

    void
    handle_net_restack_window(void)
    {
    }

    void
    handle_net_request_frame_extents(void)
    {
    }

  private:
    // x::connection & m_c;
    // x::registry & m_registry;
    // xcb_cursor_t m_cursor;
    // uint8_t m_button;
    // std::shared_ptr<motion> m_motion;
}; // class ewmh

}; // namespace zen

#endif // ZEN_EWMH_HPP

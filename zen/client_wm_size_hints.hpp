#ifndef ZEN_CLIENT_WM_SIZE_HINTS_HPP
#define ZEN_CLIENT_WM_SIZE_HINTS_HPP

#include <xcb/xcb_icccm.h>

#include "../zen/interface.hpp"

namespace zen {

namespace client {

class wm_size_hints : public interface::client {
  public:
    wm_size_hints(interface::client_ptr client) : interface::client(client)
    {
      auto reply = m_c.get_property(false, m_window, XCB_ATOM_WM_NORMAL_HINTS,
                                    XCB_ATOM_WM_SIZE_HINTS, 0,
                                    XCB_ICCCM_NUM_WM_SIZE_HINTS_ELEMENTS);

      xcb_size_hints_t hints;
      uint8_t status = xcb_icccm_get_wm_size_hints_from_reply(
          &hints, const_cast<xcb_get_property_reply_t *>(*reply));

      if (status) {
        m_width_inc  = hints.width_inc;
        m_height_inc = hints.height_inc;
      }
    }

    virtual client & width(unsigned int width)
    {
      return m_client->width(
          (width / m_width_inc) * m_width_inc + m_width_inc);
    }

    virtual client & height(unsigned int height)
    {
      return m_client->height(
          (height / m_height_inc) * m_height_inc + m_height_inc);
    }

  private:
    unsigned int m_width_inc = 0;
    unsigned int m_height_inc = 0;

}; // class wm_size_hint

}; // namespace client

}; // namespace zen

#endif // ZEN_CLIENT_WM_SIZE_HINTS_HPP

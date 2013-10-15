#ifndef ZEN_CLIENT_WM_SIZE_HINTS_HPP
#define ZEN_CLIENT_WM_SIZE_HINTS_HPP

#include <xcb/xcb_icccm.h>

#include "../zen/interface.hpp"

namespace zen {

namespace client {

class wm_size_hints : public interface::client {
  public:
    wm_size_hints(interface::client::ptr client) : interface::client(client)
    {
      auto reply = m_c.get_property(false, m_window, XCB_ATOM_WM_NORMAL_HINTS,
                                    XCB_ATOM_WM_SIZE_HINTS, 0,
                                    XCB_ICCCM_NUM_WM_SIZE_HINTS_ELEMENTS);

      xcb_icccm_get_wm_size_hints_from_reply(
          &m_hints, const_cast<xcb_get_property_reply_t *>(*reply));
    }

    virtual client & x(int x)
    {
      m_x += x;
      return *this;
    }

    virtual client & y(int y)
    {
      m_y += y;
      return *this;
    }

    virtual client & width(int width)
    {
      m_width += width;
      return *this;
    }

    virtual client & height(int height)
    {
      m_height += height;
      return *this;
    }

    virtual client & configure(void)
    {
      if (m_width >= m_hints.width_inc || m_width <= -m_hints.width_inc) {

        if (m_x != 0) m_client->x(-m_width);

        m_client->width(m_width);
        m_x = 0;
        m_width = 0;
      }

      if (m_height >= m_hints.height_inc || m_height <= -m_hints.height_inc) {

        if (m_y != 0) m_client->y(-m_height);

        m_client->height(m_height);
        m_y = 0;
        m_height = 0;
      }

      m_client->configure();
      return *this;
    }

  private:
    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;

    xcb_size_hints_t m_hints;

}; // class wm_size_hint

}; // namespace client

}; // namespace zen

#endif // ZEN_CLIENT_WM_SIZE_HINTS_HPP

#ifndef X_CLIENT_HPP
#define X_CLIENT_HPP

#include <iostream>
#include <climits>
#include <memory>

#include "../x/window.hpp"
#include "../x/interface.hpp"

namespace zen {

using namespace x;

class client;

typedef std::shared_ptr<client> client_ptr;

class client : public window
             , public interface::event::dispatcher
             , public interface::event::sink<xcb_map_request_event_t>
             , public interface::event::sink<xcb_button_press_event_t>
             , public interface::event::sink<xcb_motion_notify_event_t>
             , public interface::event::sink<xcb_configure_request_event_t>
             {
  public:
    friend std::ostream & operator<<(std::ostream &, const client &);

    client(connection & c, interface::event::source & s, xcb_window_t w)
      : window(c, w), m_s(s)
    {
      s.insert(this);
      auto reply = get_attributes();
      if (! reply->override_redirect) {
        change_attributes(XCB_CW_EVENT_MASK, { XCB_EVENT_MASK_KEY_PRESS
                                             | XCB_EVENT_MASK_KEY_RELEASE
                                             });

        grab_button(false,
                    XCB_EVENT_MASK_BUTTON_PRESS
                    | XCB_EVENT_MASK_BUTTON_RELEASE
                    | XCB_EVENT_MASK_BUTTON_MOTION,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE,
                    XCB_BUTTON_INDEX_1, XCB_MOD_MASK_ANY);

      }
    }

    ~client(void)
    {
      m_s.remove(this);

      ungrab_key(XCB_GRAB_ANY, XCB_MOD_MASK_ANY);
      ungrab_button(XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);
    }

    priority_masks
    masks(void)
    {
      return { { UINT_MAX, XCB_CONFIGURE_REQUEST }
             , { UINT_MAX, XCB_BUTTON_PRESS }
             , { UINT_MAX, XCB_BUTTON_RELEASE }
             , { UINT_MAX, XCB_MOTION_NOTIFY }
             , { UINT_MAX, XCB_MAP_REQUEST }
             };
    }

    void
    handle(xcb_map_request_event_t * e)
    {
      if (e->window != m_window) return;
      map();
    }

    void
    handle(xcb_configure_request_event_t * e)
    {
      if (e->window != m_window) return;

      std::vector<uint32_t> values;

      if (e->value_mask & XCB_CONFIG_WINDOW_X)            values.push_back(e->x);
      if (e->value_mask & XCB_CONFIG_WINDOW_Y)            values.push_back(e->y);
      if (e->value_mask & XCB_CONFIG_WINDOW_WIDTH)        values.push_back(e->width);
      if (e->value_mask & XCB_CONFIG_WINDOW_HEIGHT)       values.push_back(e->height);
      if (e->value_mask & XCB_CONFIG_WINDOW_BORDER_WIDTH) values.push_back(e->border_width);
      if (e->value_mask & XCB_CONFIG_WINDOW_SIBLING)      values.push_back(e->sibling);
      if (e->value_mask & XCB_CONFIG_WINDOW_STACK_MODE)   values.push_back(e->stack_mode);

      configure(e->value_mask, values);
    }

    void
    handle(xcb_button_press_event_t * e)
    {
      if (e->event != m_window) return;

      if ((e->response_type & ~0x80) == XCB_BUTTON_PRESS) {
        m_pointer_offset_x = e->event_x;
        m_pointer_offset_y = e->event_y;
      }
    }

    void
    handle(xcb_motion_notify_event_t * e)
    {
      if (e->event != m_window) return;

      configure(XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
                { static_cast<uint32_t>(e->root_x - m_pointer_offset_x),
                  static_cast<uint32_t>(e->root_y - m_pointer_offset_y) });
    }

  private:
    interface::event::source & m_s;

    unsigned int m_pointer_offset_x;
    unsigned int m_pointer_offset_y;
}; // class client

std::ostream & operator<<(std::ostream & os, const client & c)
{
  return os << c.id();
}

}; // namespace zen

#endif // X_CLIENT_HPP

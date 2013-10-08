#ifndef X_CLIENT_HPP
#define X_CLIENT_HPP

#include <iostream>
#include <climits>
#include <cmath>
#include <memory>

#include "../x/window.hpp"
#include "../x/interface.hpp"

namespace zen {

namespace client {

namespace event = x::interface::event;

class client;

typedef std::shared_ptr<client> client_ptr;

class client : public x::window
             , public event::dispatcher
             , public event::sink<xcb_enter_notify_event_t>
             , public event::sink<xcb_focus_in_event_t>
             , public event::sink<xcb_map_request_event_t>
             , public event::sink<xcb_configure_request_event_t>
             {
  public:
    friend std::ostream & operator<<(std::ostream &, const client &);

    client(x::connection & c, event::source & s, xcb_window_t w)
      : x::window(c, w), m_s(s)
    {
      s.insert(this);
      auto reply = get_attributes();
      if (! reply->override_redirect) {
        change_attributes(XCB_CW_BORDER_PIXEL | XCB_CW_EVENT_MASK,
                          { 0x000000ff
                          , XCB_EVENT_MASK_KEY_PRESS
                          | XCB_EVENT_MASK_KEY_RELEASE
                          | XCB_EVENT_MASK_ENTER_WINDOW
                          | XCB_EVENT_MASK_LEAVE_WINDOW
                          | XCB_EVENT_MASK_FOCUS_CHANGE
                          });

        configure(XCB_CONFIG_WINDOW_BORDER_WIDTH, { 1 });

        grab_button(false,
                    XCB_EVENT_MASK_BUTTON_PRESS,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE,
                    XCB_BUTTON_INDEX_1, XCB_MOD_MASK_4);

        grab_button(false,
                    XCB_EVENT_MASK_BUTTON_PRESS,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE,
                    XCB_BUTTON_INDEX_3, XCB_MOD_MASK_4);
      }
    }

    ~client(void)
    {
      m_s.remove(this);

    }

    priority_masks
    masks(void)
    {
      return { { UINT_MAX, XCB_CONFIGURE_REQUEST }
             , { UINT_MAX, XCB_MAP_REQUEST }
             , { UINT_MAX, XCB_ENTER_NOTIFY }
             , { UINT_MAX, XCB_LEAVE_NOTIFY }
             , { UINT_MAX, XCB_FOCUS_IN }
             , { UINT_MAX, XCB_FOCUS_OUT }
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
    handle(xcb_enter_notify_event_t * e)
    {
      if (e->event != m_window) return;

      if ((e->response_type & ~0x80) == XCB_ENTER_NOTIFY) {
        change_attributes(XCB_CW_BORDER_PIXEL, { 0x00ff0000 });
      } else { // XCB_LEAVE_NOTIFY
        change_attributes(XCB_CW_BORDER_PIXEL, { 0x000000ff });
      }
    }

    void
    handle(xcb_focus_in_event_t * e)
    {
      return;
      if (e->event != m_window) return;

      if ((e->response_type & ~0x80) == XCB_FOCUS_IN) {
        change_attributes(XCB_CW_BORDER_PIXEL, { 0x00ff0000 });
      } else { // XCB_FOCUS_OUT
        change_attributes(XCB_CW_BORDER_PIXEL, { 0x000000ff });
      }
    }

  private:
    event::source & m_s;
}; // class client

std::ostream & operator<<(std::ostream & os, const client & c)
{
  return os << c.id();
}

}; // namespace client

}; // namespace zen

#endif // X_CLIENT_HPP

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
                    XCB_EVENT_MASK_BUTTON_PRESS,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE,
                    XCB_BUTTON_INDEX_1, XCB_MOD_MASK_4);

        grab_button(false,
                    XCB_EVENT_MASK_BUTTON_PRESS,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE,
                    XCB_BUTTON_INDEX_3, XCB_MOD_MASK_4);


        xcb_font_t font = m_c.generate_id();
        m_c.open_font(font, "cursor");

        m_move_cursor = m_c.generate_id();
        m_c.create_glyph_cursor(m_move_cursor, font, font, 52, 52 + 1,
                                0, 0, 0, 0xffff, 0xffff, 0xffff);

        m_resize_cursor = m_c.generate_id();
        m_c.create_glyph_cursor(m_resize_cursor, font, font, 14, 14 + 1,
                                0, 0, 0, 0xffff, 0xffff, 0xffff);

        m_c.close_font(font);
      }
    }

    ~client(void)
    {
      m_s.remove(this);

      ungrab_key(XCB_GRAB_ANY, XCB_MOD_MASK_ANY);
      ungrab_button(XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);

      m_c.free_cursor(m_move_cursor);
      m_c.free_cursor(m_resize_cursor);
    }

    priority_masks
    masks(void)
    {
      return { { UINT_MAX, XCB_CONFIGURE_REQUEST }
             , { UINT_MAX, XCB_BUTTON_PRESS }
             , { UINT_MAX, XCB_BUTTON_RELEASE }
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

    /**
     * Handles move and resize actions
     */
    void
    handle(xcb_button_press_event_t * e)
    {
      if (e->event != m_window) return;

      auto start = [&](xcb_cursor_t cursor) {
        m_s.insert({ { 0, XCB_MOTION_NOTIFY } }, this);
        *(m_c.grab_pointer(false, m_window,
                           XCB_EVENT_MASK_BUTTON_MOTION
                           | XCB_EVENT_MASK_BUTTON_RELEASE,
                           XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                           XCB_NONE, cursor, XCB_TIME_CURRENT_TIME));
      };

      auto finish = [&]() {
        m_c.ungrab_pointer(XCB_TIME_CURRENT_TIME);
        m_s.remove({ { 0, XCB_MOTION_NOTIFY } }, this);
      };

      if ((e->response_type & ~0x80) == XCB_BUTTON_PRESS) {
        if (e->detail == XCB_BUTTON_INDEX_1) {
          m_move = true;
          m_pointer_offset_x = e->event_x;
          m_pointer_offset_y = e->event_y;

          start(m_move_cursor);

        } else if (e->detail == XCB_BUTTON_INDEX_3) {
          m_resize = true;

          start(m_resize_cursor);
        }

      } else { // XCB_BUTTON_RELEASE
        finish();
        if (e->detail == XCB_BUTTON_INDEX_1) {
          m_move = false;

        } else if (e->detail == XCB_BUTTON_INDEX_3) {
          m_resize = false;
        }

      }
    }

    void
    handle(xcb_motion_notify_event_t * e)
    {
      if (e->event != m_window) return;

      if (m_move) {
        configure(XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
                  { static_cast<uint32_t>(e->root_x - m_pointer_offset_x),
                    static_cast<uint32_t>(e->root_y - m_pointer_offset_y) });

      } else if (m_resize) {
        configure(XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                  { static_cast<uint32_t>(e->event_x),
                    static_cast<uint32_t>(e->event_y) });
      }
    }

  private:
    interface::event::source & m_s;

    xcb_cursor_t m_move_cursor;
    xcb_cursor_t m_resize_cursor;

    bool m_move = false;
    bool m_resize = false;
    unsigned int m_pointer_offset_x = 0;
    unsigned int m_pointer_offset_y = 0;
}; // class client

std::ostream & operator<<(std::ostream & os, const client & c)
{
  return os << c.id();
}

}; // namespace zen

#endif // X_CLIENT_HPP

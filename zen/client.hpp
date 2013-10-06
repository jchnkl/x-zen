#ifndef X_CLIENT_HPP
#define X_CLIENT_HPP

#include <iostream>
#include <climits>
#include <cmath>
#include <memory>

#include "../x/window.hpp"
#include "../x/interface.hpp"

namespace zen {

namespace event = x::interface::event;

class client;

typedef std::shared_ptr<client> client_ptr;

class client : public x::window
             , public event::dispatcher
             , public event::sink<xcb_enter_notify_event_t>
             , public event::sink<xcb_focus_in_event_t>
             , public event::sink<xcb_map_request_event_t>
             , public event::sink<xcb_button_press_event_t>
             , public event::sink<xcb_motion_notify_event_t>
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


        xcb_font_t font = m_c.generate_id();
        m_c.open_font(font, "cursor");

        m_move_cursor = m_c.generate_id();
        m_c.create_glyph_cursor(m_move_cursor, font, font, 52, 52 + 1,
                                0, 0, 0, 0xffff, 0xffff, 0xffff);

        m_resize_cursor_top = m_c.generate_id();
        m_c.create_glyph_cursor(m_resize_cursor_top, font, font,
                                138, 138 + 1, 0, 0, 0, 0xffff, 0xffff, 0xffff);

        m_resize_cursor_bottom = m_c.generate_id();
        m_c.create_glyph_cursor(m_resize_cursor_bottom, font, font,
                                16, 16 + 1, 0, 0, 0, 0xffff, 0xffff, 0xffff);

        m_resize_cursor_left = m_c.generate_id();
        m_c.create_glyph_cursor(m_resize_cursor_left, font, font,
                                70, 70 + 1, 0, 0, 0, 0xffff, 0xffff, 0xffff);

        m_resize_cursor_right = m_c.generate_id();
        m_c.create_glyph_cursor(m_resize_cursor_right, font, font,
                                96, 96 + 1, 0, 0, 0, 0xffff, 0xffff, 0xffff);

        m_resize_cursor_topleft = m_c.generate_id();
        m_c.create_glyph_cursor(m_resize_cursor_topleft, font, font,
                                134, 134 + 1, 0, 0, 0, 0xffff, 0xffff, 0xffff);

        m_resize_cursor_topright = m_c.generate_id();
        m_c.create_glyph_cursor(m_resize_cursor_topright, font, font,
                                136, 136 + 1, 0, 0, 0, 0xffff, 0xffff, 0xffff);

        m_resize_cursor_bottomleft = m_c.generate_id();
        m_c.create_glyph_cursor(m_resize_cursor_bottomleft, font, font,
                                12, 12 + 1, 0, 0, 0, 0xffff, 0xffff, 0xffff);

        m_resize_cursor_bottomright = m_c.generate_id();
        m_c.create_glyph_cursor(m_resize_cursor_bottomright, font, font,
                                14, 14 + 1, 0, 0, 0, 0xffff, 0xffff, 0xffff);

        m_c.close_font(font);
      }
    }

    ~client(void)
    {
      m_s.remove(this);

      ungrab_key(XCB_GRAB_ANY, XCB_MOD_MASK_ANY);
      ungrab_button(XCB_BUTTON_INDEX_ANY, XCB_MOD_MASK_ANY);

      m_c.free_cursor(m_move_cursor);
      m_c.free_cursor(m_resize_cursor_top);
      m_c.free_cursor(m_resize_cursor_bottom);
      m_c.free_cursor(m_resize_cursor_left);
      m_c.free_cursor(m_resize_cursor_right);
      m_c.free_cursor(m_resize_cursor_topleft);
      m_c.free_cursor(m_resize_cursor_topright);
      m_c.free_cursor(m_resize_cursor_bottomleft);
      m_c.free_cursor(m_resize_cursor_bottomright);
    }

    priority_masks
    masks(void)
    {
      return { { UINT_MAX, XCB_CONFIGURE_REQUEST }
             , { UINT_MAX, XCB_BUTTON_PRESS }
             , { UINT_MAX, XCB_BUTTON_RELEASE }
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

    /**
     * Handles move and resize actions
     */
    void
    handle(xcb_button_press_event_t * e)
    {
      if (e->event != m_window) return;

      if ((e->response_type & ~0x80) == XCB_BUTTON_PRESS) {
        if (e->detail == XCB_BUTTON_INDEX_1) {
          m_move = true;
          m_pointer_position_x = e->event_x;
          m_pointer_position_y = e->event_y;

          begin_motion(m_move_cursor);

        } else if (e->detail == XCB_BUTTON_INDEX_3) {
          m_resize = true;

          m_pointer_position_x = e->event_x;
          m_pointer_position_y = e->event_y;

          auto reply = get_geometry();
          m_old_width = reply->width;
          m_old_height = reply->height;
          warp_pointer(reply->width, reply->height);

          begin_motion(m_resize_cursor);
        }

      } else { // XCB_BUTTON_RELEASE
        end_motion();
        if (e->detail == XCB_BUTTON_INDEX_1) {
          m_move = false;

        } else if (e->detail == XCB_BUTTON_INDEX_3) {
          m_resize = false;

          auto reply = get_geometry();
          warp_pointer(m_pointer_position_x
                         * (reply->width / (double)m_old_width),
                       m_pointer_position_y
                         * (reply->height / (double)m_old_height));
        }

      }
    }

    void
    handle(xcb_motion_notify_event_t * e)
    {
      if (e->event != m_window) return;

      if (m_move) {
        configure(XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
                  { static_cast<uint32_t>(e->root_x - m_pointer_position_x),
                    static_cast<uint32_t>(e->root_y - m_pointer_position_y) });

      } else if (m_resize) {
        configure(XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                  { static_cast<uint32_t>(e->event_x),
                    static_cast<uint32_t>(e->event_y) });
      }
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

    xcb_cursor_t m_move_cursor;
    xcb_cursor_t m_resize_cursor_top;
    xcb_cursor_t m_resize_cursor_bottom;
    xcb_cursor_t m_resize_cursor_left;
    xcb_cursor_t m_resize_cursor_right;
    xcb_cursor_t m_resize_cursor_topleft;
    xcb_cursor_t m_resize_cursor_topright;
    xcb_cursor_t m_resize_cursor_bottomleft;
    xcb_cursor_t m_resize_cursor_bottomright;

    bool m_move = false;
    bool m_resize = false;
    unsigned int m_old_width = 0;
    unsigned int m_old_height = 0;
    unsigned int m_pointer_position_x = 0;
    unsigned int m_pointer_position_y = 0;

    void
    begin_motion(xcb_cursor_t cursor)
    {
      m_s.insert({ { 0, XCB_MOTION_NOTIFY } }, this);
      *(m_c.grab_pointer(false, m_window,
                         XCB_EVENT_MASK_BUTTON_MOTION
                         | XCB_EVENT_MASK_BUTTON_RELEASE,
                         XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                         XCB_NONE, cursor, XCB_TIME_CURRENT_TIME));
    }

    void
    end_motion(void)
    {
      m_c.ungrab_pointer(XCB_TIME_CURRENT_TIME);
      m_s.remove({ { 0, XCB_MOTION_NOTIFY } }, this);
    }
}; // class client

std::ostream & operator<<(std::ostream & os, const client & c)
{
  return os << c.id();
}

}; // namespace zen

#endif // X_CLIENT_HPP

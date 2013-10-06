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

          begin_motion();

        } else if (e->detail == XCB_BUTTON_INDEX_3) {
          m_resize = true;

          m_pointer_position_x = e->event_x;
          m_pointer_position_y = e->event_y;

          m_origin_x = e->root_x;
          m_origin_y = e->root_y;

          auto reply = get_geometry();
          m_old_x = reply->x;
          m_old_y = reply->y;
          m_old_width = reply->width;
          m_old_height = reply->height;

          double norm_x = (double)(e->event_x - reply->width / 2) / (reply->width / 2);
          double norm_y = (double)(e->event_y - reply->height / 2) / (reply->height / 2);

          double angle = (180 / M_PI) * (M_PI - std::atan2(norm_x, norm_y));

          // 360 / 8 = 45; 45 / 2 = 22.5
                 if (angle >  22.5 && angle <=  67.5) {
            warp_pointer(m_old_width, 0);
            m_origin_x = reply->x + m_old_width;
            m_origin_y = reply->y;
            m_resize_direction = TOPRIGHT;

          } else if (angle >  67.5 && angle <= 112.5) {
            warp_pointer(m_old_width, m_old_height / 2);
            m_origin_x = reply->x + m_old_width;
            m_origin_y = reply->y + m_old_height / 2;
            m_resize_direction = RIGHT;

          } else if (angle > 112.5 && angle <= 157.5) {
            warp_pointer(m_old_width, m_old_height);
            m_origin_x = reply->x + m_old_width;
            m_origin_y = reply->y + m_old_height;
            m_resize_direction = BOTTOMRIGHT;

          } else if (angle > 157.5 && angle <= 202.5) {
            warp_pointer(m_old_width / 2, m_old_height);
            m_origin_x = reply->x + m_old_width / 2;
            m_origin_y = reply->y + m_old_height;
            m_resize_direction = BOTTOM;

          } else if (angle > 202.5 && angle <= 247.5) {
            warp_pointer(0, m_old_height);
            m_origin_x = reply->x;
            m_origin_y = reply->y + m_old_height;
            m_resize_direction = BOTTOMLEFT;

          } else if (angle > 247.5 && angle <= 292.5) {
            warp_pointer(0, m_old_height / 2);
            m_origin_x = reply->x;
            m_origin_y = reply->y + m_old_height / 2;
            m_resize_direction = LEFT;

          } else if (angle > 292.5 && angle <= 337.5) {
            warp_pointer(0, 0);
            m_origin_x = reply->x;
            m_origin_y = reply->y;
            m_resize_direction = TOPLEFT;

          } else {
            warp_pointer(m_old_width / 2, 0);
            m_origin_x = reply->x + m_old_width / 2;
            m_origin_y = reply->y;
            m_resize_direction = TOP;

          }

                 begin_motion(true);
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
        uint32_t mask = 0;
        std::vector<uint32_t> values;

        switch (m_resize_direction) {
          case TOP:
            mask = XCB_CONFIG_WINDOW_Y
                 | XCB_CONFIG_WINDOW_HEIGHT;
            values.push_back(m_old_y + e->root_y - m_origin_y);
            values.push_back(m_old_height + m_origin_y - e->root_y);
            break;

          case TOPRIGHT:
            mask = XCB_CONFIG_WINDOW_Y
                 | XCB_CONFIG_WINDOW_WIDTH
                 | XCB_CONFIG_WINDOW_HEIGHT;
            values.push_back(m_old_y + e->root_y - m_origin_y);
            values.push_back(m_old_width + e->root_x - m_origin_x);
            values.push_back(m_old_height + m_origin_y - e->root_y);
            break;

          case RIGHT:
            mask = XCB_CONFIG_WINDOW_WIDTH;
            values.push_back(m_old_width + e->root_x - m_origin_x);
            break;

          case BOTTOMRIGHT:
            mask = XCB_CONFIG_WINDOW_WIDTH
                 | XCB_CONFIG_WINDOW_HEIGHT;
            values.push_back(m_old_width + e->root_x - m_origin_x);
            values.push_back(m_old_height + e->root_y - m_origin_y);
            break;

          case BOTTOM:
            mask = XCB_CONFIG_WINDOW_HEIGHT;
            values.push_back(m_old_height + e->root_y - m_origin_y);
            break;

          case BOTTOMLEFT:
            mask = XCB_CONFIG_WINDOW_X
                 | XCB_CONFIG_WINDOW_WIDTH
                 | XCB_CONFIG_WINDOW_HEIGHT;
            values.push_back(m_old_x + e->root_x - m_origin_x);
            values.push_back(m_old_width + m_origin_x - e->root_x);
            values.push_back(m_old_height + e->root_y - m_origin_y);
            break;

          case LEFT:
            mask = XCB_CONFIG_WINDOW_X
                 | XCB_CONFIG_WINDOW_WIDTH;
            values.push_back(m_old_x + e->root_x - m_origin_x);
            values.push_back(m_old_width + m_origin_x - e->root_x);
            break;

          case TOPLEFT:
            mask = XCB_CONFIG_WINDOW_X
                 | XCB_CONFIG_WINDOW_Y
                 | XCB_CONFIG_WINDOW_WIDTH
                 | XCB_CONFIG_WINDOW_HEIGHT;
            values.push_back(m_old_x + e->root_x - m_origin_x);
            values.push_back(m_old_y + e->root_y - m_origin_y);
            values.push_back(m_old_width + m_origin_x - e->root_x);
            values.push_back(m_old_height + m_origin_y - e->root_y);
            break;
        }

        configure(mask, values);
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
    enum direction { LEFT, RIGHT, TOP, BOTTOM,
                     TOPLEFT, TOPRIGHT, BOTTOMLEFT, BOTTOMRIGHT };

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
    unsigned int m_old_x = 0;
    unsigned int m_old_y = 0;
    unsigned int m_origin_x = 0;
    unsigned int m_origin_y = 0;
    unsigned int m_old_width = 0;
    unsigned int m_old_height = 0;
    unsigned int m_pointer_position_x = 0;
    unsigned int m_pointer_position_y = 0;

    direction m_resize_direction;

    void
    begin_motion(bool resize = false)
    {
      xcb_cursor_t cursor = m_move_cursor;

      if (resize) {
      switch (m_resize_direction) {
        case TOP:         cursor = m_resize_cursor_top;         break;
        case TOPRIGHT:    cursor = m_resize_cursor_topright;    break;
        case RIGHT:       cursor = m_resize_cursor_right;       break;
        case BOTTOMRIGHT: cursor = m_resize_cursor_bottomright; break;
        case BOTTOM:      cursor = m_resize_cursor_bottom;      break;
        case BOTTOMLEFT:  cursor = m_resize_cursor_bottomleft;  break;
        case LEFT:        cursor = m_resize_cursor_left;        break;
        case TOPLEFT:     cursor = m_resize_cursor_topleft;     break;
      }
      }

      *(m_c.grab_pointer(false, m_window,
                         XCB_EVENT_MASK_BUTTON_MOTION
                         | XCB_EVENT_MASK_BUTTON_RELEASE,
                         XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                         XCB_NONE, cursor, XCB_TIME_CURRENT_TIME));

      m_s.insert({ { 0, XCB_MOTION_NOTIFY } }, this);
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

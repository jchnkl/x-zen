#ifndef X_POINTER_HPP
#define X_POINTER_HPP

#include <iostream>
#include <cmath>
#include <climits>
#include <unordered_map>

#include <X11/cursorfont.h>

#include "../x/window.hpp"
#include "../x/connection.hpp"
#include "../x/interface.hpp"
#include "../zen/interface.hpp"

namespace zen {

namespace pointer {

namespace event = x::interface::event;

class cursors {
  public:
    cursors(x::connection & c) : m_c(c)
    {
      m_font = m_c.generate_id();
      m_c.open_font(m_font, "cursor");
    }

    ~cursors(void)
    {
      m_c.close_font(m_font);
      for (auto & item : m_cursors) {
        m_c.free_cursor(item.second);
      }
    }

    xcb_cursor_t
    operator[](const uint16_t & source_char)
    {
      try {
        return m_cursors.at(source_char);
      } catch (...) {
        m_cursors[source_char] = m_c.generate_id();
        m_c.create_glyph_cursor(m_cursors[source_char], m_font, m_font,
                                source_char, source_char + 1,
                                0, 0, 0, 0xffff, 0xffff, 0xffff);
        return m_cursors[source_char];
      }
    }

  private:
    x::connection & m_c;
    xcb_font_t m_font;
    std::unordered_map<uint16_t, xcb_cursor_t> m_cursors;
}; // class cursors

class resize : public event::dispatcher
             , public event::sink<xcb_button_press_event_t>
             , public event::sink<xcb_motion_notify_event_t>
             {
  public:
    resize(x::connection & c, event::source & s,
           cursors & cursors, interface::manager & manager)
      : m_c(c), m_s(s), m_cursors(cursors), m_manager(manager)
    {
      m_s.insert(this);
    }

    ~resize(void)
    {
      m_s.remove(this);
    }

    priority_masks
    masks(void)
    {
      return { { UINT_MAX, XCB_BUTTON_PRESS }
             , { UINT_MAX, XCB_BUTTON_RELEASE }
             , { UINT_MAX, XCB_MOTION_NOTIFY }
             };
    }

    void
    handle(xcb_button_press_event_t * e)
    {
      if (XCB_BUTTON_RELEASE == (e->response_type & ~0x80)) {
        m_c.ungrab_pointer(XCB_TIME_CURRENT_TIME);
        m_current_client.reset();
        return;

      } else if (XCB_BUTTON_INDEX_3 == e->detail && XCB_MOD_MASK_4 == e->state) {
        m_current_client = m_manager[e->event];

      } else {
        return;
      }

      if (! m_current_client) return;

      m_pointer_x = e->root_x;
      m_pointer_y = e->root_y;

      auto reply = m_current_client->get_geometry();

      m_origin_x = reply->x;
      m_origin_y = reply->y;
      m_origin_width = reply->width;
      m_origin_height = reply->height;

      double normalized_x = (double)(e->event_x - reply->width / 2)
                          / (reply->width / 2);
      double normalized_y = (double)(e->event_y - reply->height / 2)
                          / (reply->height / 2);
      double angle = (180 / M_PI) * (M_PI - std::atan2(normalized_x, normalized_y));

      xcb_cursor_t cursor;

      // 360 / 8 = 45; 45 / 2 = 22.5
      if (angle >  22.5 && angle <=  67.5) {
        cursor = m_cursors[XC_top_right_corner];
        m_direction = TOPRIGHT;
        m_pointer_x = reply->x + m_origin_width;
        m_pointer_y = reply->y;

      } else if (angle >  67.5 && angle <= 112.5) {
        cursor = m_cursors[XC_right_side];
        m_direction = RIGHT;
        m_pointer_x = reply->x + m_origin_width;
        m_pointer_y = reply->y + m_origin_height / 2;

      } else if (angle > 112.5 && angle <= 157.5) {
        cursor = m_cursors[XC_bottom_right_corner];
        m_direction = BOTTOMRIGHT;
        m_pointer_x = reply->x + m_origin_width;
        m_pointer_y = reply->y + m_origin_height;

      } else if (angle > 157.5 && angle <= 202.5) {
        cursor = m_cursors[XC_bottom_side];
        m_direction = BOTTOM;
        m_pointer_x = reply->x + m_origin_width / 2;
        m_pointer_y = reply->y + m_origin_height;

      } else if (angle > 202.5 && angle <= 247.5) {
        cursor = m_cursors[XC_bottom_left_corner];
        m_direction = BOTTOMLEFT;
        m_pointer_x = reply->x;
        m_pointer_y = reply->y + m_origin_height;

      } else if (angle > 247.5 && angle <= 292.5) {
        cursor = m_cursors[XC_left_side];
        m_direction = LEFT;
        m_pointer_x = reply->x;
        m_pointer_y = reply->y + m_origin_height / 2;

      } else if (angle > 292.5 && angle <= 337.5) {
        cursor = m_cursors[XC_top_left_corner];
        m_direction = TOPLEFT;
        m_pointer_x = reply->x;
        m_pointer_y = reply->y;

      } else {
        cursor = m_cursors[XC_top_side];
        m_direction = TOP;
        m_pointer_x = reply->x + m_origin_width / 2;
        m_pointer_y = reply->y;

      }

      m_current_client->warp_pointer(m_pointer_x - reply->x,
                                     m_pointer_y - reply->y);

      *(m_c.grab_pointer(false, m_current_client->id(),
                         XCB_EVENT_MASK_BUTTON_MOTION
                         | XCB_EVENT_MASK_BUTTON_RELEASE,
                         XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                         XCB_NONE, cursor, XCB_TIME_CURRENT_TIME));
    }

    void
    handle(xcb_motion_notify_event_t * e)
    {
      if (! (m_current_client && e->event == m_current_client->id())) return;

      uint32_t mask = 0;
      std::vector<uint32_t> values;

      switch (m_direction) {
        case TOP:
          mask = XCB_CONFIG_WINDOW_Y
               | XCB_CONFIG_WINDOW_HEIGHT;
          values.push_back(m_origin_y + e->root_y - m_pointer_y);
          values.push_back(m_origin_height + m_pointer_y - e->root_y);
          break;

        case TOPRIGHT:
          mask = XCB_CONFIG_WINDOW_Y
               | XCB_CONFIG_WINDOW_WIDTH
               | XCB_CONFIG_WINDOW_HEIGHT;
          values.push_back(m_origin_y + e->root_y - m_pointer_y);
          values.push_back(m_origin_width + e->root_x - m_pointer_x);
          values.push_back(m_origin_height + m_pointer_y - e->root_y);
          break;

        case RIGHT:
          mask = XCB_CONFIG_WINDOW_WIDTH;
          values.push_back(m_origin_width + e->root_x - m_pointer_x);
          break;

        case BOTTOMRIGHT:
          mask = XCB_CONFIG_WINDOW_WIDTH
               | XCB_CONFIG_WINDOW_HEIGHT;
          values.push_back(m_origin_width + e->root_x - m_pointer_x);
          values.push_back(m_origin_height + e->root_y - m_pointer_y);
          break;

        case BOTTOM:
          mask = XCB_CONFIG_WINDOW_HEIGHT;
          values.push_back(m_origin_height + e->root_y - m_pointer_y);
          break;

        case BOTTOMLEFT:
          mask = XCB_CONFIG_WINDOW_X
               | XCB_CONFIG_WINDOW_WIDTH
               | XCB_CONFIG_WINDOW_HEIGHT;
          values.push_back(m_origin_x + e->root_x - m_pointer_x);
          values.push_back(m_origin_width + m_pointer_x - e->root_x);
          values.push_back(m_origin_height + e->root_y - m_pointer_y);
          break;

        case LEFT:
          mask = XCB_CONFIG_WINDOW_X
               | XCB_CONFIG_WINDOW_WIDTH;
          values.push_back(m_origin_x + e->root_x - m_pointer_x);
          values.push_back(m_origin_width + m_pointer_x - e->root_x);
          break;

        case TOPLEFT:
          mask = XCB_CONFIG_WINDOW_X
               | XCB_CONFIG_WINDOW_Y
               | XCB_CONFIG_WINDOW_WIDTH
               | XCB_CONFIG_WINDOW_HEIGHT;
          values.push_back(m_origin_x + e->root_x - m_pointer_x);
          values.push_back(m_origin_y + e->root_y - m_pointer_y);
          values.push_back(m_origin_width + m_pointer_x - e->root_x);
          values.push_back(m_origin_height + m_pointer_y - e->root_y);
          break;
      }

      m_current_client->configure(mask, values);
    }

  private:
    enum direction { LEFT, RIGHT, TOP, BOTTOM,
                     TOPLEFT, TOPRIGHT, BOTTOMLEFT, BOTTOMRIGHT };

    x::connection & m_c;
    event::source & m_s;
    cursors & m_cursors;
    interface::manager & m_manager;

    interface::client_ptr m_current_client;
    direction m_direction;
    unsigned int m_pointer_x;
    unsigned int m_pointer_y;
    unsigned int m_origin_x;
    unsigned int m_origin_y;
    unsigned int m_origin_width;
    unsigned int m_origin_height;
}; // class resize

class move : public event::dispatcher
           , public event::sink<xcb_button_press_event_t>
           , public event::sink<xcb_motion_notify_event_t>
           {
  public:
    move(x::connection & c, event::source & s,
         cursors & cursors, interface::manager & manager)
      : m_c(c), m_s(s), m_cursors(cursors), m_manager(manager)
    {
      m_s.insert(this);
    }

    ~move(void)
    {
      m_s.remove(this);
    }

    priority_masks
    masks(void)
    {
      return { { UINT_MAX, XCB_BUTTON_PRESS }
             , { UINT_MAX, XCB_BUTTON_RELEASE }
             };
    }

    void
    handle(xcb_button_press_event_t * e)
    {
      if (XCB_BUTTON_RELEASE == (e->response_type & ~0x80)) {
        m_c.ungrab_pointer(XCB_TIME_CURRENT_TIME);
        m_s.remove({{ 0, XCB_MOTION_NOTIFY }}, this);
        m_current_client.reset();
        return;

      } else if (XCB_BUTTON_INDEX_1 == e->detail && XCB_MOD_MASK_4 == e->state) {
        m_current_client = m_manager[e->event];

      } else {
        return;
      }

      if (! m_current_client) return;

      m_pointer_position_x = e->event_x;
      m_pointer_position_y = e->event_y;

      m_s.insert({{ 0, XCB_MOTION_NOTIFY }}, this);

      *(m_c.grab_pointer(false, m_current_client->id(),
                         XCB_EVENT_MASK_BUTTON_MOTION
                         | XCB_EVENT_MASK_BUTTON_RELEASE,
                         XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                         XCB_NONE, m_cursors[XC_fleur],
                         XCB_TIME_CURRENT_TIME));
    }

    void
    handle(xcb_motion_notify_event_t * e)
    {
      if (! (m_current_client && e->event == m_current_client->id())) return;

      m_current_client->configure(
          XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
          { static_cast<uint32_t>(e->root_x - m_pointer_position_x),
            static_cast<uint32_t>(e->root_y - m_pointer_position_y) });
    }

  private:
    x::connection & m_c;
    event::source & m_s;
    cursors & m_cursors;
    interface::manager & m_manager;

    xcb_cursor_t m_cursor;
    interface::client_ptr m_current_client;
    unsigned int m_pointer_position_x;
    unsigned int m_pointer_position_y;
}; // class move

}; // namespace pointer

}; // namespace zen

#endif // X_POINTER_HPP

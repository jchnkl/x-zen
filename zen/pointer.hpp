#ifndef X_POINTER_HPP
#define X_POINTER_HPP

#include <iostream>
#include <climits>

#include "../x/cursor.hpp"
#include "../x/window.hpp"
#include "../x/connection.hpp"
#include "../x/interface.hpp"
#include "../zen/algorithm.hpp"
#include "../zen/interface.hpp"
#include "../zen/client_wm_size_hints.hpp"

namespace zen {

namespace pointer {

using zen::interface::client;
namespace button = zen::interface::button;
namespace xevent = x::interface::event;

class resize : public interface::client
             , public xevent::dispatcher
             , public xevent::sink<xcb_motion_notify_event_t>
             , public button::handler<XCB_BUTTON_INDEX_3, XCB_MOD_MASK_4>
             {
  public:
    class factory : public zen::interface::client::factory {
      public:
        factory(zen::interface::client::factory * const factory_ptr,
                x::connection & c, xevent::source & s, x::cursor & cursor)
          : zen::interface::client::factory(factory_ptr)
          , m_c(c), m_s(s), m_cursor(cursor)
        {}

        client::ptr
        make(const xcb_window_t & window, const client::ptr & c)
        {
          return std::shared_ptr<client>(new resize(c, m_c, m_s, m_cursor));
        }

      private:
        x::connection & m_c;
        xevent::source & m_s;
        x::cursor & m_cursor;

    }; // class factory

    resize(interface::client::ptr client,
           x::connection & c, xevent::source & s, x::cursor & cursor)
      : interface::client(client)
      , m_c(c), m_s(s), m_cursor(cursor)
    {}

    void
    press(xcb_button_press_event_t * const e)
    {
      using namespace algorithm;

      auto reply = m_client->get_geometry();

      m_direction = corner()(e->event_x, e->event_y,
                             reply->width, reply->height);

      xcb_cursor_t cursor = 0;

      m_pointer_x = reply->width / 2;
      m_pointer_y = reply->height / 2;

      switch (m_direction.first) {
        case TOP:
          m_pointer_y = 0;

          if (m_direction.second == LEFT) {
            cursor = m_cursor[XC_top_left_corner];
          } else if (m_direction.second == RIGHT) {
            cursor = m_cursor[XC_top_right_corner];
          } else {
            cursor = m_cursor[XC_top_side];
          }
          break;

        case BOTTOM:
          m_pointer_y = reply->height;

          if (m_direction.second == LEFT) {
            cursor = m_cursor[XC_bottom_left_corner];
          } else if (m_direction.second == RIGHT) {
            cursor = m_cursor[XC_bottom_right_corner];
          } else {
            cursor = m_cursor[XC_bottom_side];
          }
          break;

        case NONE:
          if (m_direction.second == LEFT) {
            cursor = m_cursor[XC_left_side];
          } else if (m_direction.second == RIGHT) {
            cursor = m_cursor[XC_right_side];
          }

        default:
          break;
      }

      switch (m_direction.second) {
        case LEFT:
          m_pointer_x = 0;
          break;
        case RIGHT:
          m_pointer_x = reply->width;
          break;
        case NONE:
        default:
          break;
      }

      m_client->warp_pointer(m_pointer_x, m_pointer_y);

      m_pointer_x += reply->x;
      m_pointer_y += reply->y;

      m_s.insert({{ 0, XCB_MOTION_NOTIFY }}, this);

      *(m_c.grab_pointer(false, m_client->id(),
                         XCB_EVENT_MASK_BUTTON_MOTION
                         | XCB_EVENT_MASK_BUTTON_RELEASE,
                         XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                         XCB_NONE, cursor, XCB_TIME_CURRENT_TIME));
    }

    void
    release(xcb_button_press_event_t * const e)
    {
      m_c.ungrab_pointer(XCB_TIME_CURRENT_TIME);
      m_s.remove({{ 0, XCB_MOTION_NOTIFY }}, this);
    }

    void
    handle(xcb_motion_notify_event_t * e)
    {
      using namespace algorithm;

      switch (m_direction.first) {
        case TOP:
          m_client->y(e->root_y - m_pointer_y)
                   .height(m_pointer_y - e->root_y);
          break;

        case BOTTOM:
          m_client->height(e->root_y - m_pointer_y);
          break;

        case NONE:
        default:
          break;
      };

      switch (m_direction.second) {
        case RIGHT:
          m_client->width(e->root_x - m_pointer_x);
          break;

        case LEFT:
          m_client->x(e->root_x - m_pointer_x )
                   .width(m_pointer_x - e->root_x);

          break;

        case NONE:
        default:
          break;
      };

      m_pointer_x = e->root_x;
      m_pointer_y = e->root_y;

      m_client->configure();
    }

  private:
    x::connection & m_c;
    xevent::source & m_s;
    x::cursor & m_cursor;

    std::pair<algorithm::direction, algorithm::direction> m_direction;
    unsigned int m_pointer_x;
    unsigned int m_pointer_y;

}; // class resize

class move : public interface::client
           , public xevent::dispatcher
           , public xevent::sink<xcb_motion_notify_event_t>
           , public button::handler<XCB_BUTTON_INDEX_1, XCB_MOD_MASK_4>
           {
  public:
    class factory : public zen::interface::client::factory {
      public:
        factory(zen::interface::client::factory * const factory_ptr,
                x::connection & c, xevent::source & s, x::cursor & cursor)
          : zen::interface::client::factory(factory_ptr)
          , m_c(c), m_s(s), m_cursor(cursor)
        {}

        client::ptr
        make(const xcb_window_t & window, const client::ptr & c)
        {
          return std::shared_ptr<client>(new move(c, m_c, m_s, m_cursor));
        }

      private:
        x::connection & m_c;
        xevent::source & m_s;
        x::cursor & m_cursor;

    }; // class factory

    move(interface::client::ptr client,
         x::connection & c, xevent::source & s, x::cursor & cursor)
      : interface::client(client)
      , m_c(c), m_s(s), m_cursor(cursor)
    {}

    void
    press(xcb_button_press_event_t * const e)
    {
      m_pointer_x = e->root_x;
      m_pointer_y = e->root_y;

      m_s.insert({{ 0, XCB_MOTION_NOTIFY }}, this);

      *(m_client->grab_pointer(
            false,
            XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_BUTTON_RELEASE,
            XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE,
            m_cursor[XC_fleur]));
    }

    void
    release(xcb_button_release_event_t * const e)
    {
      m_c.ungrab_pointer();
      m_s.remove({{ 0, XCB_MOTION_NOTIFY }}, this);
    }

    void
    handle(xcb_motion_notify_event_t * e)
    {
      m_client->x(e->root_x - m_pointer_x)
               .y(e->root_y - m_pointer_y)
               .configure();

      m_pointer_x = e->root_x;
      m_pointer_y = e->root_y;
    }

  private:
    x::connection & m_c;
    xevent::source & m_s;
    x::cursor & m_cursor;

    unsigned int m_pointer_x;
    unsigned int m_pointer_y;

}; // class move

}; // namespace pointer

}; // namespace zen

#endif // X_POINTER_HPP

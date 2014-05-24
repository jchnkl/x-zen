#ifndef ZEN_RESIZER_HPP
#define ZEN_RESIZER_HPP

#include "xpp/xpp.hpp"
#include "types.hpp"
#include "util.hpp"
#include "cursor.hpp"

namespace zen {

class resizer
  : public xpp::event::sink<x::button_press, x::button_release>
{
  public:
    resizer(x::connection & c,
            x::registry & registry,
            zen::cursor & cursor,
            uint8_t button)
      : m_c(c)
      , m_registry(registry)
      , m_cursor(cursor)
      , m_button(button)
    {}

    void handle(const x::button_press & e)
    {
      if (e->detail == m_button) {
        xcb_cursor_t cursor = XCB_NONE;
        auto g = e.event<zen::window>().get_geometry();
        auto direction = corner(e->event_x, e->event_y, g->width, g->height);

        if (direction.first == NORTH && direction.second == NONE) {
          cursor = m_cursor[XC_top_side];
          m_motion = std::make_shared<motion<north, none>>(
              e->root_x, e->root_y, g->x, g->y, g->width, g->height);

        } else if (direction.first == NORTH && direction.second == EAST) {
          cursor = m_cursor[XC_top_right_corner];
          m_motion = std::make_shared<motion<north, east>>(
              e->root_x, e->root_y, g->x, g->y, g->width, g->height);

        } else if (direction.first == NORTH && direction.second == WEST) {
          cursor = m_cursor[XC_top_left_corner];
          m_motion = std::make_shared<motion<north, west>>(
              e->root_x, e->root_y, g->x, g->y, g->width, g->height);

        } else if (direction.first == SOUTH && direction.second == NONE) {
          cursor = m_cursor[XC_bottom_side];
          m_motion = std::make_shared<motion<south, none>>(
              e->root_x, e->root_y, g->x, g->y, g->width, g->height);

        } else if (direction.first == SOUTH && direction.second == EAST) {
          cursor = m_cursor[XC_bottom_right_corner];
          m_motion = std::make_shared<motion<south, east>>(
              e->root_x, e->root_y, g->x, g->y, g->width, g->height);

        } else if (direction.first == SOUTH && direction.second == WEST) {
          cursor = m_cursor[XC_bottom_left_corner];
          m_motion = std::make_shared<motion<south, west>>(
              e->root_x, e->root_y, g->x, g->y, g->width, g->height);

        } else if (direction.first == NONE && direction.second == EAST) {
          cursor = m_cursor[XC_right_side];
          m_motion = std::make_shared<motion<none, east>>(
              e->root_x, e->root_y, g->x, g->y, g->width, g->height);

        } else if (direction.first == NONE && direction.second == WEST) {
          cursor = m_cursor[XC_left_side];
          m_motion = std::make_shared<motion<none, west>>(
              e->root_x, e->root_y, g->x, g->y, g->width, g->height);
        }

        m_registry.attach(0, m_motion.get());

        m_c.change_active_pointer_grab(cursor,
                                       XCB_EVENT_MASK_BUTTON_PRESS
                                       | XCB_EVENT_MASK_BUTTON_RELEASE
                                       | XCB_EVENT_MASK_BUTTON_MOTION);
      }
    }

    void handle(const x::button_release & e)
    {
      if (m_motion && e->detail == m_button) {
        m_registry.detach(0, m_motion.get());
        m_motion.reset();
      }
    }

  private:
    struct north {};
    struct south {};
    struct east {};
    struct west {};
    struct none {};

    // NS = north, south, none; EW = east, west, none
    template<typename NS, typename EW>
    struct motion
      : public xpp::event::sink<x::motion_notify>
    {
      int16_t m_root_x = 0;
      int16_t m_root_y = 0;
      int32_t m_x = 0;
      int32_t m_y = 0;
      int32_t m_width = 0;
      int32_t m_height = 0;

      uint32_t m_mask = 0;
      uint32_t m_values[4];

      motion(int16_t root_x, int16_t root_y,
             int32_t x, int32_t y, int32_t width, int32_t height)
        : m_root_x(root_x)
        , m_root_y(root_y)
        , m_x(x)
        , m_y(y)
        , m_width(width)
        , m_height(height)
      {
        set_mask(NS());
        set_mask(EW());
      }

      void
      set_mask(north)
      {
        m_mask |= XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_HEIGHT;
      }

      void
      set_mask(south)
      {
        m_mask |= XCB_CONFIG_WINDOW_HEIGHT;
      }

      void
      set_mask(east)
      {
        m_mask |= XCB_CONFIG_WINDOW_WIDTH;
      }

      void
      set_mask(west)
      {
        m_mask |= XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_WIDTH;
      }

      void
      set_mask(none)
      {}

      void
      pack(north, none)
      {
        m_values[0] = m_y;
        m_values[1] = m_height;
      }

      void
      pack(south, none)
      {
        m_values[0] = m_height;
      }

      void
      pack(none, east)
      {
        m_values[0] = m_width;
      }

      void
      pack(none, west)
      {
        m_values[0] = m_x;
        m_values[1] = m_width;
      }

      void
      pack(north, east)
      {
        m_values[0] = m_y;
        m_values[1] = m_width;
        m_values[2] = m_height;
      }

      void
      pack(north, west)
      {
        m_values[0] = m_x;
        m_values[1] = m_y;
        m_values[2] = m_width;
        m_values[3] = m_height;
      }

      void
      pack(south, east)
      {
        m_values[0] = m_width;
        m_values[1] = m_height;
      }

      void
      pack(south, west)
      {
        m_values[0] = m_x;
        m_values[1] = m_width;
        m_values[2] = m_height;
      }

      void resize(north, int16_t delta_y)
      {
        m_y += delta_y;
        m_height -= delta_y;
      }

      void resize(south, int16_t delta_y)
      {
        m_height += delta_y;
      }

      void resize(east, int16_t delta_x)
      {
        m_width += delta_x;
      }

      void resize(west, int16_t delta_x)
      {
        m_x += delta_x;
        m_width -= delta_x;
      }

      void resize(none, int16_t)
      {}

      void handle(const x::motion_notify & e)
      {
        resize(NS(), e->root_y - m_root_y);
        resize(EW(), e->root_x - m_root_x);

        pack(NS(), EW());

        e.event<x::window>().configure(m_mask, m_values);

        m_root_x = e->root_x;
        m_root_y = e->root_y;
      }
    };

    x::connection & m_c;
    x::registry & m_registry;
    zen::cursor & m_cursor;
    uint8_t m_button;
    std::shared_ptr<xpp::event::sink<x::motion_notify>> m_motion;
}; // class resizer

}; // namespace zen

#endif // ZEN_RESIZER_HPP

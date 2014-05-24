#ifndef ZEN_MOVER_HPP
#define ZEN_MOVER_HPP

#include "xpp/xpp.hpp"
#include "types.hpp"
#include "cursor.hpp"

namespace zen {

class mover
  : public xpp::event::sink<x::button_press, x::button_release>
{
  public:
    mover(x::connection & c,
          x::registry & registry,
          zen::cursor & cursor,
          uint8_t button)
      : m_c(c)
      , m_registry(registry)
      , m_cursor(cursor[XC_fleur])
      , m_button(button)
    {}

    void handle(const x::button_press & e)
    {
      if (e->detail == m_button) {
        m_motion = std::make_shared<motion>();
        m_motion->m_event_x = e->event_x;
        m_motion->m_event_y = e->event_y;
        m_registry.attach(0, m_motion.get());

        m_c.change_active_pointer_grab(m_cursor,
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
    struct motion
      : public xpp::event::sink<x::motion_notify>
    {
      int16_t m_event_x;
      int16_t m_event_y;

      void handle(const x::motion_notify & e)
      {
        uint32_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
        uint32_t values[] = { (uint32_t)e->root_x - m_event_x,
                              (uint32_t)e->root_y - m_event_y };
        e.event<x::window>().configure(mask, values);
      }
    };

    x::connection & m_c;
    x::registry & m_registry;
    xcb_cursor_t m_cursor;
    uint8_t m_button;
    std::shared_ptr<motion> m_motion;
}; // class mover

}; // namespace zen

#endif // ZEN_MOVER_HPP

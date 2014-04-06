#ifndef ZEN_EWMH_HPP
#define ZEN_EWMH_HPP

#include "xpp/xpp.hpp"
#include "types.hpp"
#include "cursor.hpp"

namespace zen {

class ewmh
  : public xpp::event::sink<x::property_notify>
{
  public:
    // ewmh(x::connection & c,
    //       x::registry & registry,
    //       zen::cursor & cursor,
    //       uint8_t button)
    //   : m_c(c)
    //   , m_registry(registry)
    //   , m_cursor(cursor[XC_fleur])
    //   , m_button(button)
    // {}

    void handle(const x::property_notify & e)
    {
    }

  private:
    // x::connection & m_c;
    // x::registry & m_registry;
    // xcb_cursor_t m_cursor;
    // uint8_t m_button;
    // std::shared_ptr<motion> m_motion;
}; // class ewmh

}; // namespace zen

#endif // ZEN_EWMH_HPP

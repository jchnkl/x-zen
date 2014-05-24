#ifndef ZEN_RAISER_HPP
#define ZEN_RAISER_HPP

#include "xpp/xpp.hpp"
#include "types.hpp"

namespace zen {

class raiser
  : public xpp::event::sink<x::button_press>
{
  public:
    raiser(const uint8_t & button)
      : m_button(button)
    {}

    void handle(const x::button_press & e)
    {
      if (e->detail == m_button) {
        uint32_t mask = XCB_CONFIG_WINDOW_STACK_MODE;
        uint32_t values[] = { XCB_STACK_MODE_BELOW };
        e.event<x::window>().configure(mask, values);
      }
    }

  private:
    uint8_t m_button;
}; // class raiser

}; // namespace zen

#endif // ZEN_RAISER_HPP

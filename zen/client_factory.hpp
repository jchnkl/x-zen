#ifndef ZEN_CLIENT_FACTORY_HPP
#define ZEN_CLIENT_FACTORY_HPP

#include "../x/cursor.hpp"
#include "../x/interface.hpp"
#include "../zen/client.hpp"
#include "../zen/interface.hpp"

namespace zen {

namespace xevent = x::interface::event;

namespace client {

class factory : public zen::interface::client::factory {
  public:
    factory(x::connection & c, xevent::source & s)
      : m_c(c), m_s(s)
    {}

    client::ptr
    make(const xcb_window_t & window, const client::ptr & c)
    {
      return std::shared_ptr<interface::client>(
          new client(m_c, m_s, window));
    }

  private:
    x::connection & m_c;
    xevent::source & m_s;

}; // class factory

}; // namespace client

}; // namespace zen

#endif // ZEN_CLIENT_FACTORY_HPP

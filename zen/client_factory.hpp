#ifndef ZEN_CLIENT_FACTORY_HPP
#define ZEN_CLIENT_FACTORY_HPP

#include "../x/cursor.hpp"
#include "../x/interface.hpp"
#include "../zen/client.hpp"
#include "../zen/pointer.hpp"
#include "../zen/interface.hpp"

namespace zen {

namespace client {

using zen::interface::event;
namespace xevent = x::interface::event;

class factory : public interface::client::factory {
  public:
    factory(x::connection & c, xevent::source & s, x::cursor & cursor)
      : m_c(c), m_s(s), m_cursor(cursor)
    {}

    interface::client::ptr make(const xcb_window_t & window)
    {
      auto c = std::shared_ptr<interface::client>(
          new client(m_c, m_s, window));

      c = std::shared_ptr<interface::client>(
          new pointer::move(c, m_c, m_s, m_cursor));

      c = std::shared_ptr<interface::client>(
          new pointer::resize(c, m_c, m_s, m_cursor));

      return c;
    }

  private:
    x::connection & m_c;
    xevent::source & m_s;
    x::cursor & m_cursor;

}; // class factory

}; // namespace zen

}; // namespace client

#endif // ZEN_CLIENT_FACTORY_HPP

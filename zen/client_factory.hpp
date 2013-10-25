#ifndef ZEN_CLIENT_FACTORY_HPP
#define ZEN_CLIENT_FACTORY_HPP

#include "../x/interface.hpp"
#include "../zen/client.hpp"
#include "../zen/interface.hpp"

namespace zen {

namespace client {

using zen::interface::event;
namespace xevent = x::interface::event;

class factory : public interface::client::factory {
  public:
    factory(x::connection & c, xevent::source & s)
      : m_c(c), m_s(s)
    {}

    interface::client::ptr make(const xcb_window_t & window)
    {
      auto new_client =
        std::shared_ptr<client>(
            new client(m_c, m_s,
                       window));

      return new_client;
    }






    }

  private:
    x::connection & m_c;
    xevent::source & m_s;

}; // class factory

}; // namespace zen

}; // namespace client

#endif // ZEN_CLIENT_FACTORY_HPP

#ifndef ZEN_CLIENT_FACTORY_HPP
#define ZEN_CLIENT_FACTORY_HPP

#include "../x/interface.hpp"
#include "../zen/client.hpp"
#include "../zen/interface.hpp"

namespace zen {

namespace client {

using zen::interface::event;
namespace key = zen::interface::key;
namespace button = zen::interface::button;
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
                       m_key_event_handler,
                       m_button_event_handler,
                       window));

      for (auto & handler : m_key_handlers) {
        new_client->insert(handler);
      }

      for (auto & handler : m_button_handlers) {
        new_client->insert(handler);
      }

      return new_client;
    }

    void
    key_event_handler(event<xcb_key_press_event_t> * const h)
    {
      m_key_event_handler = h;
    }

    void
    button_event_handler(event<xcb_button_press_event_t> * const h)
    {
      m_button_event_handler = h;
    }

    void
    insert(key::handler * const h)
    {
      m_key_handlers.insert(h);
    }

    void
    insert(button::handler * const h)
    {
      m_button_handlers.insert(h);
    }

    void
    remove(key::handler * const h)
    {
      m_key_handlers.erase(h);
    }

    void
    remove(button::handler * const h)
    {
      m_button_handlers.erase(h);
    }

  private:
    x::connection & m_c;
    xevent::source & m_s;

    event<xcb_key_press_event_t> * m_key_event_handler;
    event<xcb_button_press_event_t> * m_button_event_handler;

    std::unordered_set<key::handler *> m_key_handlers;
    std::unordered_set<button::handler *> m_button_handlers;

}; // class factory

}; // namespace zen

}; // namespace client

#endif // ZEN_CLIENT_FACTORY_HPP

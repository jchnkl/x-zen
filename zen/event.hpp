#ifndef ZEN_EVENT_HPP
#define ZEN_EVENT_HPP

#include <climits>
#include "../generic_accessor.hpp"
#include "interface.hpp"

MAKE_ACCESSOR(window, event);
MAKE_ACCESSOR(window, window);

namespace zen {

namespace event {

template<typename E>
class event : public x::interface::event::dispatcher
            , public x::interface::event::sink<E>
            , public zen::interface::event<E>
            {
  public:
    event(zen::interface::manager & manager,
          x::interface::event::source & s,
          const std::vector<x::interface::event::type> & types)
      : m_manager(manager), m_s(s)
    {
      for (auto & type : types) {
        m_masks.push_back({ UINT_MAX, type });
      }
      m_s.insert(m_masks, this);
    }

    ~event(void)
    {
      m_s.remove(m_masks, this);
    }

    void
    insert(zen::interface::handler<E> * h) {
      handlers.insert(h);
    }

    void
    remove(zen::interface::handler<E> * h) {
      handlers.erase(h);
    }

    void
    handle(E * e)
    {
      auto client = m_manager[get_window(e)];
      if (client) {
        for (auto & h : handlers) {
          h->handle(client, e);
        }
      }
    }

  protected:
    priority_masks m_masks;
    zen::interface::manager & m_manager;
    x::interface::event::source & m_s;
    std::unordered_set<zen::interface::handler<E> *> handlers;

}; // class event

}; // namespace event

}; // namespace zen

#endif // ZEN_EVENT_HPP

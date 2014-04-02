#ifndef ZEN_EVENT_HPP
#define ZEN_EVENT_HPP

#include <climits>
#include "../generic_accessor.hpp"
#include "interface.hpp"

MAKE_ACCESSOR(window, event);
MAKE_ACCESSOR(window, window);

namespace zen {

// event object

// template<typename E>
// class sink {
//   public:
//     virtual void handle(event<E> * const e) = 0;
// };

class container {
  public:
    virtual x::interface::event::dispatcher * const
      operator[](const xcb_window_t &) = 0;
}; // class container

template<typename E, xcb_window_t (*F)(E *)>
class direct_event {
  public:

    direct_event(const container & c)
      : m_container(c)
    {}

    void handle(E * e)
    {
      try {
        m_container[F(e)]->dispatch(e);
      } catch (...) {}
    }

  private:
    const container & m_container;
};

void foo(void)
{
  container * c;
  direct_event<xcb_key_press_event_t, get_window> de(*c);
}

namespace event {

template<typename E, typename W>
class object {
  public:
    E & operator*() { return *m_event; }
    E * const operator->() { return m_event; }
    W * const window() { return m_window; }

  private:
    E * m_event = NULL;
    W * m_window = NULL;
}; // class event

template<typename E>
class source : public x::interface::event::dispatcher
            , public x::interface::event::sink<E>
            , public zen::interface::event<E>
            {
  public:
    source(const container & container,
          x::interface::event::source & s,
          const std::vector<x::interface::event::type> & types)
      : m_container(container), m_s(s)
    {
      for (auto & type : types) {
        m_masks.push_back({ UINT_MAX, type });
      }
      m_s.insert(m_masks, this);
    }

    ~source(void)
    {
      m_s.remove(m_masks, this);
    }

    void
    handle(E * e)
    {
      try {
        m_container[get_window(e)]->dispatch(e);
      } catch (...) {}
    }

  protected:
    priority_masks m_masks;
    // zen::interface::manager & m_manager;
    x::interface::event::source & m_s;
    const container & m_container;

}; // class event

}; // namespace event

// event with window

/*
template<typename H>
class event_dispatcher {
  public:
    void attach(H * const h)
    {
      m_handlers.insert(h);
    }

    void detach(H * const h)
    {
      m_handlers.erase(h);
    }

    template<typename E>
      void handle(E * const e)
    {
      for (auto & h : m_handlers) {
        h->handle(e);
      }
    }

  protected:
    std::unordered_multiset<H *> m_handlers;
};

template<typename ... HANDLER>
class event_registry : event_dispatcher<HANDLER> ... {
  public:
    client_event(x::interface::event::source & s) : m_s(s) {}

    template<typename H> void
      attach(H * const h)
    {
      static_cast<client_event_dispatcher<H> *>(this)->attach(h);
    }

    template<typename H> void
      detach(H * const h)
    {
      static_cast<client_event_dispatcher<H> *>(this)->detach(h);
    }

    template<typename E> void
      handle(E * const e)
    {
      static_cast<client_event_dispatcher<client::handler<E>> *>(this)->accept(e);
    }

  private:
    x::interface::event::source & m_s;
};
*/

}; // namespace zen

#endif // ZEN_EVENT_HPP

#ifndef ZEN_EVENT_HPP
#define ZEN_EVENT_HPP

#include <climits>
#include "interface.hpp"

#define CREATE_MEMBER_DETECTOR(X, I)                                                \
template<typename T> class Detect_##X {                                             \
    struct Fallback { int X; };                                                     \
    struct Derived : T, Fallback { };                                               \
                                                                                    \
    template<typename U, U> struct Check;                                           \
                                                                                    \
    typedef char ArrayOfOne[1];                                                     \
    typedef char ArrayOfTwo[2];                                                     \
                                                                                    \
    template<typename U> static ArrayOfOne & func(Check<int Fallback::*, &U::X> *); \
    template<typename U> static ArrayOfTwo & func(...);                             \
  public:                                                                           \
    typedef Detect_##X type;                                                        \
    enum { value = sizeof(func<Derived>(0)) == 2                                    \
                   ? (int)I : (int)member_type::none };                             \
};

namespace zen {

namespace event {

enum class member_type { none = 0, event, window };

CREATE_MEMBER_DETECTOR(event, member_type::event)
CREATE_MEMBER_DETECTOR(window, member_type::window)

template<int, typename E>
struct event_window {
  static xcb_window_t get_member(E * e);
};

template<typename E>
struct event_window<(int)member_type::none, E> {
  static xcb_window_t get(E * e)
  {
    return 0;
  }
};

template<typename E>
struct event_window<(int)member_type::event, E> {
  static xcb_window_t get(E * e)
  {
    return e->event;
  }
};

template<typename E>
struct event_window<(int)member_type::window, E> {
  static xcb_window_t get(E * e)
  {
    return e->window;
  }
};

template<typename E>
xcb_window_t get_event_window(E * e)
{
  xcb_window_t window = 0;

  window = event_window<Detect_event<E>::value, E>::get(e);
  if (0 != window) return window;

  window = event_window<Detect_window<E>::value, E>::get(e);
  if (0 != window) return window;

  return window;
}

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
      auto client = m_manager[get_event_window(e)];
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

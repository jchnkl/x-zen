#ifndef ZEN_CLIENT_MANAGER_HPP
#define ZEN_CLIENT_MANAGER_HPP

#include <iostream>
#include <algorithm>
#include <deque>
#include <unordered_map>

#include "client.hpp"

namespace zen {

namespace client {

namespace event = x::interface::event;

class manager
  , public event::dispatcher
  , public event::sink<xcb_create_notify_event_t>
  , public event::sink<xcb_destroy_notify_event_t>
{
  public:
    typedef std::deque<xcb_window_t> window_deque;
    typedef std::unordered_map<xcb_window_t, client_ptr> window_client_map;

    friend std::ostream & operator<<(std::ostream &, const manager &);

    manager(connection & c, event::source & s)
      : m_c(c), m_s(s)
    {
      s.insert(this);
    }

    ~manager(void)
    {
      m_s.remove(this);
    }

    priority_masks
    masks(void)
    {
      return { { UINT_MAX, XCB_CREATE_NOTIFY }
             , { UINT_MAX, XCB_DESTROY_NOTIFY } };
    }

    void
    handle(xcb_create_notify_event_t * e)
    {
      insert(e->window);
    }

    void
    handle(xcb_destroy_notify_event_t * e)
    {
      remove(e->window);
    }

    void
    insert(xcb_window_t window)
    {
      m_client_order.push_front(window);
      m_clients[window] = client_ptr(new client(m_c, m_s, window));
    }

    void
    remove(xcb_window_t window)
    {
      m_client_order.erase(
          std::find(m_client_order.begin(), m_client_order.end(), window));
      m_clients.erase(window);
    }

    const window_client_map &
    clients(void)
    {
      return m_clients;
    }

  private:
    connection & m_c;
    event::source & m_s;

    window_deque m_client_order;
    window_client_map m_clients;
}; // class client_manager

std::ostream & operator<<(std::ostream & os, const manager & cm)
{
  std::size_t i = 0;
  for (i = 0; i < cm.m_client_order.size(); ++i) {
    os << *cm.m_clients.at(cm.m_client_order[i]);

    if (i < cm.m_client_order.size() - 1) {
      os << ", ";
    }
  }

  return os;
}

}; // namespace zen

#endif // ZEN_CLIENT_MANAGER_HPP

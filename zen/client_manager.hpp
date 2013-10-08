#ifndef ZEN_CLIENT_MANAGER_HPP
#define ZEN_CLIENT_MANAGER_HPP

#include <iostream>
#include <algorithm>
#include <deque>
#include <unordered_map>

#include "client.hpp"

namespace zen {

using namespace x;

class client_manager
  : public interface::event::dispatcher
  , public interface::event::sink<xcb_create_notify_event_t>
  , public interface::event::sink<xcb_destroy_notify_event_t>
{
  public:
    friend std::ostream & operator<<(std::ostream &, const client_manager &);

    client_manager(connection & c, interface::event::source & s)
      : m_c(c), m_s(s)
    {
      s.insert(this);
    }

    ~client_manager(void)
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

    const std::unordered_map<xcb_window_t, client_ptr> &
    clients(void)
    {
      return m_clients;
    }

  private:
    connection & m_c;
    interface::event::source & m_s;

    std::deque<xcb_window_t> m_client_order;
    std::unordered_map<xcb_window_t, client_ptr> m_clients;
}; // class client_manager

std::ostream & operator<<(std::ostream & os, const client_manager & cm)
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

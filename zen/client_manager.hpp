#ifndef X_CLIENT_MANAGER_HPP
#define X_CLIENT_MANAGER_HPP

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
      client_order.push_front(window);
      clients[window] = client_ptr(new client(m_c, m_s, window));
    }

    void
    remove(xcb_window_t window)
    {
      client_order.erase(
          std::find(client_order.begin(), client_order.end(), window));
      clients.erase(window);
    }

  private:
    connection & m_c;
    interface::event::source & m_s;

    std::deque<xcb_window_t> client_order;
    std::unordered_map<xcb_window_t, client_ptr> clients;
}; // class client

}; // namespace zen

#endif // X_CLIENT_MANAGER_HPP

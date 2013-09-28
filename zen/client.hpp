#ifndef X_CLIENT_HPP
#define X_CLIENT_HPP

#include <iostream>
#include <climits>
#include <memory>

#include "../x/window.hpp"
#include "../x/interface.hpp"

namespace zen {

using namespace x;

class client;

typedef std::shared_ptr<client> client_ptr;

class client : public window
             , public interface::event::dispatcher
             , public interface::event::sink<xcb_configure_notify_event_t>
             {
  public:
    friend std::ostream & operator<<(std::ostream &, const client &);

    client(connection & c, interface::event::source & s, xcb_window_t w)
      : window(c, w), m_s(s)
    {
      s.insert(this);
    }

    ~client(void)
    {
      m_s.remove(this);
    }

    priority_masks
    masks(void)
    {
      return { { UINT_MAX, XCB_CONFIGURE_NOTIFY } };
    }

    void
    handle(xcb_configure_notify_event_t * e)
    {
    }

  private:
    interface::event::source & m_s;
}; // class client

std::ostream & operator<<(std::ostream & os, const client & c)
{
  return os << c.id();
}

}; // namespace zen

#endif // X_CLIENT_HPP

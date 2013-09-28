#ifndef X_CLIENT_HPP
#define X_CLIENT_HPP

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
    client(connection & c, xcb_window_t w)
      : window(c, w)
    {}

    virtual priority_masks
    masks(void)
    {
      return { { UINT_MAX, XCB_CONFIGURE_NOTIFY } };
    }

    virtual void
    handle(xcb_configure_notify_event_t * e)
    {
    }

  private:
}; // class client

}; // namespace zen

#endif // X_CLIENT_HPP

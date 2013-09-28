#ifndef X_CLIENT_HPP
#define X_CLIENT_HPP

#include "../x/window.hpp"

namespace x {

class client : public window {
  public:
    client(connection & c, xcb_window_t w)
      : window(c, w)
    {}

  private:
}; // class client

}; // namespace x

#endif // X_CLIENT_HPP

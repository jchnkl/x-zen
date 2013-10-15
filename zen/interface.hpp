#ifndef ZEN_INTERFACE_HPP
#define ZEN_INTERFACE_HPP

#include <iostream>
#include <memory>

#include "../x/window.hpp"

namespace zen {

namespace interface {

class client;

typedef std::shared_ptr<client> client_ptr;

class client : public x::window {
  public:
    client(x::connection & c, xcb_window_t w)
      : x::window(c, w)
    {}

    client(client_ptr client)
      : x::window(client->m_c, client->m_window),  m_client(client)
    {}

    virtual ~client(void) {}

    virtual int x(void)
    {
      return m_client->x();
    }

    virtual int y(void)
    {
      return m_client->y();
    }

    virtual unsigned int width(void)
    {
      return m_client->width();
    }

    virtual unsigned int height(void)
    {
      return m_client->height();
    }

    virtual unsigned int border_width(void)
    {
      return m_client->border_width();
    }

    virtual xcb_window_t sibling(void)
    {
      return m_client->sibling();
    }

    virtual xcb_stack_mode_t stack_mode(void)
    {
      return m_client->stack_mode();
    }

    virtual client & x(int x)
    {
      return m_client->x(x);
    }

    virtual client & y(int y)
    {
      return m_client->y(y);
    }

    virtual client & width(int width)
    {
      return m_client->width(width);
    }

    virtual client & height(int height)
    {
      return m_client->height(height);
    }

    virtual client & border_width(unsigned int border_width)
    {
      return m_client->border_width(border_width);
    }

    virtual client & sibling(xcb_window_t sibling)
    {
      return m_client->sibling(sibling);
    }

    virtual client & stack_mode(xcb_stack_mode_t stack_mode)
    {
      return m_client->stack_mode(stack_mode);
    }

    virtual client & configure(void)
    {
      return m_client->configure();
    }

  protected:
    client_ptr m_client;

}; // class client

class manager {
  public:
    virtual ~manager(void) {}
    virtual client_ptr operator[](const xcb_window_t &) = 0;
};

}; // namespace zen

}; // namespace interface

#endif // ZEN_INTERFACE_HPP

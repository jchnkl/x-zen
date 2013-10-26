#ifndef ZEN_CLIENT_HPP
#define ZEN_CLIENT_HPP

#include <iostream>
#include <climits>
#include <cmath>
#include <memory>
#include <unordered_set>

#include "../x/window.hpp"
#include "../x/interface.hpp"
#include "../zen/interface.hpp"

namespace zen {

namespace client {

using zen::interface::event;
namespace xevent = x::interface::event;

class client : public interface::client
             , public xevent::dispatcher
             , public xevent::sink<xcb_enter_notify_event_t>
             , public xevent::sink<xcb_focus_in_event_t>
             , public xevent::sink<xcb_map_request_event_t>
             , public xevent::sink<xcb_configure_request_event_t>
             {
  public:
    client(x::connection & c, xevent::source & s,
           const xcb_window_t & w)
      : interface::client(c, w), m_s(s)
    {
      m_s.insert(m_priority_masks, this);

      auto reply = get_attributes();
      if (! reply->override_redirect) {
        change_attributes(XCB_CW_BORDER_PIXEL | XCB_CW_EVENT_MASK,
                          { 0x000000ff
                          , XCB_EVENT_MASK_ENTER_WINDOW
                          | XCB_EVENT_MASK_LEAVE_WINDOW
                          | XCB_EVENT_MASK_FOCUS_CHANGE
                          });

        window::configure(XCB_CONFIG_WINDOW_BORDER_WIDTH, { 1 });

        auto reply = get_geometry();
        m_x = reply->x;
        m_y = reply->y;
        m_width = reply->width;
        m_height = reply->height;

        grab_button(false,
                    XCB_EVENT_MASK_BUTTON_PRESS,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE,
                    XCB_BUTTON_INDEX_1, XCB_MOD_MASK_4);

        grab_button(false,
                    XCB_EVENT_MASK_BUTTON_PRESS,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE,
                    XCB_BUTTON_INDEX_3, XCB_MOD_MASK_4);
      }
    }

    ~client(void)
    {
      m_s.remove(m_priority_masks, this);
    }

    // TODO: this needs to be a decorator
    void
    handle(xcb_map_request_event_t * e)
    {
      if (e->window != x::window::m_window) return;
      map();
    }

    // TODO: this needs to be a decorator
    void
    handle(xcb_configure_request_event_t * e)
    {
      if (e->window != m_window) return;

      std::vector<uint32_t> values;

      if (e->value_mask & XCB_CONFIG_WINDOW_X) {
        m_x = e->x;
        values.push_back(e->x);
      }

      if (e->value_mask & XCB_CONFIG_WINDOW_Y) {
        m_y = e->y;
        values.push_back(e->y);
      }

      if (e->value_mask & XCB_CONFIG_WINDOW_WIDTH) {
        m_width = e->width;
        values.push_back(e->width);
      }

      if (e->value_mask & XCB_CONFIG_WINDOW_HEIGHT) {
        m_height = e->height;
        values.push_back(e->height);
      }

      if (e->value_mask & XCB_CONFIG_WINDOW_BORDER_WIDTH) values.push_back(e->border_width);
      if (e->value_mask & XCB_CONFIG_WINDOW_SIBLING)      values.push_back(e->sibling);
      if (e->value_mask & XCB_CONFIG_WINDOW_STACK_MODE)   values.push_back(e->stack_mode);

      window::configure(e->value_mask, values);
    }

    // TODO: this needs to be a decorator
    void
    handle(xcb_enter_notify_event_t * e)
    {
      if (e->event != m_window) return;

      if ((e->response_type & ~0x80) == XCB_ENTER_NOTIFY) {
        change_attributes(XCB_CW_BORDER_PIXEL, { 0x00ff0000 });
      } else { // XCB_LEAVE_NOTIFY
        change_attributes(XCB_CW_BORDER_PIXEL, { 0x000000ff });
      }
    }

    // TODO: this needs to be a decorator
    void
    handle(xcb_focus_in_event_t * e)
    {
      return;
      if (e->event != m_window) return;

      if ((e->response_type & ~0x80) == XCB_FOCUS_IN) {
        change_attributes(XCB_CW_BORDER_PIXEL, { 0x00ff0000 });
      } else { // XCB_FOCUS_OUT
        change_attributes(XCB_CW_BORDER_PIXEL, { 0x000000ff });
      }
    }

    client & focus(xcb_input_focus_t revert_to = XCB_INPUT_FOCUS_PARENT)
    {
      window::focus(revert_to);
      return *this;
    }

    virtual int x(void)                       { return m_x; }
    virtual int y(void)                       { return m_y; }
    virtual unsigned int width(void)          { return m_width; }
    virtual unsigned int height(void)         { return m_height; }
    virtual unsigned int border_width(void)   { return m_border_width; }
    virtual xcb_window_t sibling(void)        { return m_sibling; }
    virtual xcb_stack_mode_t stack_mode(void) { return m_stack_mode; }

    virtual client & x(int x)
    {
      m_mask |= XCB_CONFIG_WINDOW_X;
      m_x += x;
      return *this;
    }

    virtual client & y(int y)
    {
      m_mask |= XCB_CONFIG_WINDOW_Y;
      m_y += y;
      return *this;
    }

    virtual client & width(int width)
    {
      m_mask |= XCB_CONFIG_WINDOW_WIDTH;
      m_width += width;
      return *this;
    }

    virtual client & height(int height)
    {
      m_mask |= XCB_CONFIG_WINDOW_HEIGHT;
      m_height += height;
      return *this;
    }

    virtual client & border_width(unsigned int border_width)
    {
      m_mask |= XCB_CONFIG_WINDOW_BORDER_WIDTH;
      m_border_width = border_width;
      return *this;
    }

    virtual client & sibling(xcb_window_t sibling)
    {
      m_mask |= XCB_CONFIG_WINDOW_SIBLING;
      m_sibling = sibling;
      return *this;
    }

    virtual client & stack_mode(xcb_stack_mode_t stack_mode)
    {
      m_mask |= XCB_CONFIG_WINDOW_STACK_MODE;
      m_stack_mode = stack_mode;
      return *this;
    }

    virtual client & configure(void)
    {
      std::vector<uint32_t> values;

      if (XCB_CONFIG_WINDOW_X            & m_mask) { values.push_back(m_x); }
      if (XCB_CONFIG_WINDOW_Y            & m_mask) { values.push_back(m_y); }
      if (XCB_CONFIG_WINDOW_WIDTH        & m_mask) { values.push_back(m_width); }
      if (XCB_CONFIG_WINDOW_HEIGHT       & m_mask) { values.push_back(m_height); }
      if (XCB_CONFIG_WINDOW_BORDER_WIDTH & m_mask) { values.push_back(m_border_width); }
      if (XCB_CONFIG_WINDOW_SIBLING      & m_mask) { values.push_back(m_sibling); }
      if (XCB_CONFIG_WINDOW_STACK_MODE   & m_mask) { values.push_back(m_stack_mode); }

      window::configure(m_mask, values);

      m_mask = 0;

      return *this;
    }

  private:
    xevent::source & m_s;

    priority_masks m_priority_masks =
      { { UINT_MAX, XCB_CONFIGURE_REQUEST }
      , { UINT_MAX, XCB_MAP_REQUEST }
      , { UINT_MAX, XCB_ENTER_NOTIFY }
      , { UINT_MAX, XCB_LEAVE_NOTIFY }
      , { UINT_MAX, XCB_FOCUS_IN }
      , { UINT_MAX, XCB_FOCUS_OUT }
      };

    unsigned int m_mask = 0;

    int m_x = 0;
    int m_y = 0;
    unsigned int m_width = 0;
    unsigned int m_height = 0;
    unsigned int m_border_width = 0;
    xcb_window_t m_sibling = 0;
    xcb_stack_mode_t m_stack_mode;

}; // class client

}; // namespace client

}; // namespace zen

#endif // ZEN_CLIENT_HPP

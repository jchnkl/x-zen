#ifndef ZEN_MANAGER_HPP
#define ZEN_MANAGER_HPP

#include <deque>

#include "xpp/xpp.hpp"
#include "types.hpp"

namespace zen {

class manager
  : public xpp::event::sink<x::create_notify,
                            x::destroy_notify,
                            x::map_request,
                            x::configure_request>
{
  public:
    manager(x::connection & c)
      : m_c(c)
    {
      using namespace xpp::x::reply::checked;
      using attributes = get_window_attributes<x::connection &>;
      using window_attributes_t = std::pair<x::window, attributes>;

      auto && root = c.root<x::window>();
      std::vector<window_attributes_t> window_attributes;

      for (auto && window : root.query_tree().children<x::window>()) {
        window_attributes.push_back({ window, window.get_attributes() });
      }

      for (auto && pair : window_attributes) {
        if (! (pair.second->override_redirect
              || pair.second->map_state == XCB_MAP_STATE_UNVIEWABLE) )
        {
          insert(pair.first);
        }
      }
    }

    void insert(const zen::window & window)
    {
      m_windows.push_front(window);

          std::cerr << "grab_mask: " << (int)(XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_BUTTON_MOTION) << std::endl;

      auto grab = [&](uint32_t button)
      {
        m_c.grab_button(false, window,
                        XCB_EVENT_MASK_BUTTON_PRESS
                        | XCB_EVENT_MASK_BUTTON_RELEASE
                        | XCB_EVENT_MASK_BUTTON_MOTION
                        ,
                        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                        window, XCB_NONE,
                        button, XCB_MOD_MASK_1);
      };

      grab(XCB_BUTTON_INDEX_1);
      grab(XCB_BUTTON_INDEX_2);
      grab(XCB_BUTTON_INDEX_3);

      // grab(XCB_BUTTON_MASK_1);
      // grab(XCB_BUTTON_MASK_2);
      // grab(XCB_BUTTON_MASK_3);

      {
        uint32_t mask = XCB_CONFIG_WINDOW_BORDER_WIDTH;
        uint32_t values[] = { 3 };
        window.configure(mask, values);
      }

      {
        uint32_t mask = XCB_CW_BORDER_PIXEL;
        uint32_t values[] = { 0xdaa520 };
        window.change_attributes(mask, values);
      }
    }

    void remove(const zen::window & window)
    {
      for (auto wit = m_windows.begin(); wit != m_windows.end(); ) {
        if (*wit == window) {
          wit = m_windows.erase(wit);
          break;
        } else {
          ++wit;
        }
      }
    }

    void handle(const x::create_notify & e)
    {
      insert(e.window<zen::window>());
    }

    void handle(const x::destroy_notify & e)
    {
      remove(e.window<zen::window>());
    }

    void handle(const x::map_request & e)
    {
      std::cerr << e.description() << " (" << (int)e.opcode() << ", " << (int)e->response_type << ")" << std::endl;
      e.window<x::window>().map();
    }

    void handle(const x::configure_request & e)
    {
      std::cerr << e.description() << " (" << (int)e.opcode() << ", " << (int)e->response_type << ")" << std::endl;
      std::vector<uint32_t> values;

      if (e->value_mask & XCB_CONFIG_WINDOW_X) {
        values.push_back(e->x);
      }
      if (e->value_mask & XCB_CONFIG_WINDOW_Y) {
        values.push_back(e->y);
      }
      if (e->value_mask & XCB_CONFIG_WINDOW_WIDTH) {
        values.push_back(e->width);
      }
      if (e->value_mask & XCB_CONFIG_WINDOW_HEIGHT) {
        values.push_back(e->height);
      }
      if (e->value_mask & XCB_CONFIG_WINDOW_BORDER_WIDTH) {
        values.push_back(e->border_width);
      }
      if (e->value_mask & XCB_CONFIG_WINDOW_SIBLING) {
        values.push_back(e->sibling);
      }
      if (e->value_mask & XCB_CONFIG_WINDOW_STACK_MODE) {
        values.push_back(e->stack_mode);
      }

      e.window<x::window>().configure(e->value_mask, values.data());
    }

    /*
    void handle(const x::property_notify & e)
    {
      auto name = m_c.get_atom_name(e->atom);
      std::cerr << e.description() << ":" << " name: " << name.name() << std::endl;
                // << std::hex
                // << " event::window: 0x" << e->atom
                // << std::dec
                // << " @ " << e->x
                // << "x" << e->y
                // << "+" << e->width
                // << "+" << e->height
                // << std::endl;
    }

    void handle(const x::client_message & e)
    {
      auto name = m_c.get_atom_name(e->type);
      std::cerr << e.description() << ":" << " name: " << name.name() << std::endl;
                // << std::hex
                // << " event::window: 0x" << e->atom
                // << std::dec
                // << " @ " << e->x
                // << "x" << e->y
                // << "+" << e->width
                // << "+" << e->height
                // << std::endl;
    }
    */

  private:
    x::connection & m_c;
    std::deque<zen::window> m_windows;
}; // class manager

}; // namespace zen

#endif // ZEN_MANAGER_HPP

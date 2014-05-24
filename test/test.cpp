#include <cmath> // atan2
#include <deque>

#include "../xpp/src/xpp.hpp"
#include "../xpp/src/proto/render.hpp"
#include "../xpp/src/proto/composite.hpp"
#include "types.hpp"

namespace zen {

class mover
  : public xpp::event::sink<x::button_press, x::button_release>
{
  public:
    mover(const uint8_t & button, x::registry & registry)
      : m_button(button)
      , m_registry(registry)
    {}

    void handle(const x::button_press & e)
    {
      if (e->detail == m_button) {
        m_motion = std::make_shared<motion>();
        m_motion->m_event_x = e->event_x;
        m_motion->m_event_y = e->event_y;
        m_registry.attach(0, m_motion.get());
      }
    }

    void handle(const x::button_release & e)
    {
      if (m_motion && e->detail == m_button) {
        m_registry.detach(0, m_motion.get());
        m_motion.reset();
      }
    }

  private:
    struct motion
      : public xpp::event::sink<x::motion_notify>
    {
      int16_t m_event_x;
      int16_t m_event_y;

      void handle(const x::motion_notify & e)
      {
        uint32_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
        uint32_t values[] = { (uint32_t)e->root_x - m_event_x,
                              (uint32_t)e->root_y - m_event_y };
        e.event<x::window>().configure(mask, values);
      }
    };

    uint8_t m_button;
    x::registry & m_registry;
    std::shared_ptr<motion> m_motion;
}; // class mover

class raiser
  : public xpp::event::sink<x::button_press>
{
  public:
    raiser(const uint8_t & button)
      : m_button(button)
    {}

    void handle(const x::button_press & e)
    {
      if (e->detail == m_button) {
        uint32_t mask = XCB_CONFIG_WINDOW_STACK_MODE;
        uint32_t values[] = { XCB_STACK_MODE_BELOW };
        e.event<x::window>().configure(mask, values);
      }
    }

  private:
    uint8_t m_button;
}; // class raiser

class manager
  : public xpp::event::sink<x::create_notify,
                            x::destroy_notify,
                            x::map_request,
                            x::configure_request
                            // x::expose,
                            // x::motion_notify,
                            // x::button_press,
                            // x::button_release
                            // x::property_notify,
                            // x::client_message
                           >
{
  public:
    manager(x::connection & c)
      : m_c(c)
    {}

    void insert(const zen::window & window)
    {
      std::cerr << "insert: " << window;
      m_windows.push_front(window);
      std::cerr << " [m_windows.size(): " << m_windows.size() << "]" << std::endl;

      m_c.grab_button(false, window,
                      XCB_EVENT_MASK_BUTTON_PRESS
                      | XCB_EVENT_MASK_BUTTON_RELEASE
                      | XCB_EVENT_MASK_BUTTON_MOTION,
                      XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                      window, XCB_NONE,
                      XCB_BUTTON_MASK_1, XCB_MOD_MASK_1);

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
          std::cerr << "remove: " << *wit;
          wit = m_windows.erase(wit);
          std::cerr << " [m_windows.size(): " << m_windows.size() << "]" << std::endl;
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

    /*
    void handle(const x::expose & e)
    {
      std::cerr << e.description() << ":"
                << std::hex
                << " event::window: 0x" << e->window
                << std::dec
                << " @ " << e->x
                << "x" << e->y
                << "+" << e->width
                << "+" << e->height
                << std::endl;
    }
    */

    void handle(const x::map_request & e)
    {
      std::cerr << e.description() << ": " << e.window<x::window>() << std::endl;
      e.window<x::window>().map();
    }

    void handle(const x::configure_request & e)
    {
      std::cerr << e.description() << ": " << e.window<x::window>() << std::endl;
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

int main(int, char **)
{
  try {
    x::connection connection;
    x::registry registry(connection);
    zen::manager manager(connection);
    zen::mover mover(XCB_BUTTON_INDEX_1, registry);
    zen::resizer resizer(XCB_BUTTON_INDEX_2, registry);
    zen::raiser raiser(XCB_BUTTON_INDEX_3);

    registry.attach(0, &manager);
    registry.attach(0, &mover);
    registry.attach(0, &resizer);
    registry.attach(0, &raiser);

    auto && root = connection.root<x::window>();

    {
      uint32_t mask = XCB_CW_EVENT_MASK;
      uint32_t values[] = { XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
                          | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY };
      connection.change_window_attributes_checked(root, mask, values);
    }

    std::vector<
      std::pair<x::window,
                xpp::x::reply::checked::get_window_attributes<x::connection &>>>
                  window_attributes;

    for (auto && window : root.query_tree().children<x::window>()) {
      // window_attributes.push_back(std::make_pair(window, window.get_attributes()));
      window_attributes.push_back({ window, window.get_attributes() });
    }

    for (auto && pair : window_attributes) {
      if (! (pair.second->override_redirect
            || pair.second->map_state == XCB_MAP_STATE_UNVIEWABLE) )
      {
        std::cerr << "client: " << pair.first << std::endl;
        manager.insert(pair.first);
      }
    }

    while (true) {
      connection.flush();
      registry.dispatch(connection.wait_for_event());
    }

  } catch (const std::exception & error) {
    std::cerr << "Exception (std::exception) in "
              << __FILE__ << " @ line " << __LINE__ << ", what(): "
              << error.what() << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // return EXIT_SUCCESS;
  std::exit(EXIT_SUCCESS);
}

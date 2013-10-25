#include "x/connection.hpp"
#include "x/event.hpp"
#include "x/cursor.hpp"
#include "zen/event.hpp"
#include "zen/client_factory.hpp"
#include "zen/client_manager.hpp"
#include "zen/pointer.hpp"

int main(int argc, char ** argv)
{
  x::connection c("");
  c.change_attributes(c.root(), XCB_CW_EVENT_MASK,
                      { XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
                      | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
                      });

  x::event::source source(c);

  x::cursor cursor(c);

  zen::client::factory client_factory(c, source);

  zen::client::manager client_manager(c, source, client_factory);

  zen::event::event<xcb_key_press_event_t>
    key_event_handler(client_manager, source,
                      { XCB_KEY_PRESS, XCB_KEY_RELEASE });

  zen::event::event<xcb_button_press_event_t>
    button_event_handler(client_manager, source,
                         { XCB_BUTTON_PRESS, XCB_BUTTON_RELEASE });

  zen::pointer::move move(c, source, cursor);

  zen::pointer::resize resize(c, source, cursor);

  client_factory.insert(&move);
  client_factory.insert(&resize);

  for (auto & window : c.query_tree(c.root())) {
    client_manager.insert(window);
  }

  source.run();

  return 0;
}

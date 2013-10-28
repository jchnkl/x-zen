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

  zen::interface::client::factory * factory;
  factory = new zen::client::factory(c, source);
  factory = new zen::pointer::move::factory(factory, c, source, cursor);
  factory = new zen::pointer::resize::factory(factory, c, source, cursor);

  zen::client::manager client_manager(c, source, *factory);


  zen::event::event<xcb_button_press_event_t>
    button_event_handler(client_manager, source,
                         { XCB_BUTTON_PRESS, XCB_BUTTON_RELEASE });

  for (auto & window : c.query_tree(c.root())) {
    client_manager.insert(window);
  }

  source.run();

  return 0;
}

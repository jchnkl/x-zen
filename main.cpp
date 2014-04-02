#include "x/connection.hpp"
#include "x/event.hpp"
#include "x/cursor.hpp"
#include "zen/event.hpp"
#include "zen/client_factory.hpp"
#include "zen/client_manager.hpp"
#include "zen/pointer.hpp"
#include "zen/client_snap.hpp"
#include "zen/client_wm_size_hints.hpp"

class foo : public zen::interface::handler<xcb_motion_notify_event_t> {
  public:
    void handle(xcb_motion_notify_event_t * const e) {}
};

class bar
  : public zen::interface::button::handler<XCB_BUTTON_INDEX_1, XCB_MOD_MASK_4> {
  public:
    void press(xcb_button_press_event_t * const e) {}
    void release(xcb_button_press_event_t * const e) {}
};

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
  auto snap_factory = new zen::client::snap::factory(factory);
  factory = snap_factory;
  factory = new zen::pointer::move::factory(factory, c, source, cursor);
  factory = new zen::client::wm_size_hints::factory(factory);
  factory = new zen::pointer::resize::factory(factory, c, source, cursor);

  zen::client::manager client_manager(c, source, *factory);

  snap_factory->manager(&client_manager);

  zen::event::event<xcb_button_press_event_t>
    button_event_handler(client_manager, source,
                         { XCB_BUTTON_PRESS, XCB_BUTTON_RELEASE });

  zen::client::client client(c, source, 0);
  zen::interface::client::ptr client_ptr;

  zen::pointer::move mover(client_ptr, c, source, cursor);

  client.attach(static_cast<zen::interface::button::handler<XCB_BUTTON_INDEX_1, XCB_MOD_MASK_4> *>(&mover));

  for (auto & window : c.query_tree(c.root())) {
    client_manager.insert(window);
  }

  source.run();

  return 0;
}

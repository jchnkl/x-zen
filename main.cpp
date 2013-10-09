#include "x/connection.hpp"
#include "x/event.hpp"
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

  zen::pointer::cursors cursors(c);

  zen::client::manager cm(c, source);

  for (auto & window : c.query_tree(c.root())) {
    cm.insert(window);
  }

  source.run();

  return 0;
}

#include "x/connection.hpp"
#include "x/event.hpp"
#include "zen/client_manager.hpp"

int main(int argc, char ** argv)
{
  x::connection c("");
  c.change_window_attributes(c.root(),
                             XCB_CW_EVENT_MASK,
                             { XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY });
  x::event::source s(c);

  zen::client_manager cm(c, s);

  for (auto & window : c.query_tree(c.root())) {
    cm.insert(window);
  }

  s.run();

  return 0;
}

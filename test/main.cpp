#include "types.hpp"
#include "manager.hpp"
#include "cursor.hpp"
#include "mover.hpp"
#include "resizer.hpp"
#include "raiser.hpp"

int main(int, char **)
{
  try {
    x::connection connection;
    x::registry registry(connection);
    zen::manager manager(connection);
    zen::cursor cursor(connection);
    zen::mover mover(connection, registry, cursor, XCB_BUTTON_INDEX_1);
    zen::resizer resizer(connection, registry, cursor, XCB_BUTTON_INDEX_2);
    zen::raiser raiser(XCB_BUTTON_INDEX_3);

    registry.attach(0, &manager);
    registry.attach(0, &mover);
    registry.attach(0, &resizer);
    registry.attach(0, &raiser);

    std::cerr << "root: " << std::dec << connection.root<xcb_window_t>() << std::endl;

    {
      uint32_t mask = XCB_CW_EVENT_MASK;
      uint32_t values[] = { XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
                          | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY };
      connection.root<x::window>().change_attributes_checked(mask, values);
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

  std::exit(EXIT_SUCCESS);
}

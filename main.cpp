#include "x/connection.hpp"
#include "x/event.hpp"
#include "zen/client_manager.hpp"

int main(int argc, char ** argv)
{
  x::connection c("");
  x::event::source s(c);

  zen::client_manager cm(c, s);

  s.run();

  return 0;
}

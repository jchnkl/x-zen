#include "x/connection.hpp"
#include "x/event.hpp"

int main(int argc, char ** argv)
{
  x::connection c("");
  x::event::source s(c);

  s.run();

  return 0;
}

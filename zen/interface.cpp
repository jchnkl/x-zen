#ifndef ZEN_INTERFACE_CPP
#define ZEN_INTERFACE_CPP

#include "interface.hpp"

std::ostream &
zen::interface::operator<<(std::ostream & os, client & c)
{
  return os << c.id();
}

#endif // ZEN_INTERFACE_CPP

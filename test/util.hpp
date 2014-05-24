#ifndef ZEN_UTIL_HPP
#define ZEN_UTIL_HPP

#include <cmath>
#include "types.hpp"

namespace zen {

std::pair<direction, direction>
corner(int x, int y, int width, int height)
{
  double norm_x = (double)(x - width / 2) / (width / 2);
  double norm_y = (double)(y - height / 2) / (height / 2);
  double angle = (180 / M_PI) * (M_PI - std::atan2(norm_x, norm_y));

  // 360 / 8 = 45; 45 / 2 = 22.5
  if (angle >  22.5 && angle <=  67.5) {
    return { NORTH, EAST };

  } else if (angle >  67.5 && angle <= 112.5) {
    return { NONE, EAST };

  } else if (angle > 112.5 && angle <= 157.5) {
    return { SOUTH, EAST };

  } else if (angle > 157.5 && angle <= 202.5) {
    return { SOUTH, NONE };

  } else if (angle > 202.5 && angle <= 247.5) {
    return { SOUTH, WEST };

  } else if (angle > 247.5 && angle <= 292.5) {
    return { NONE, WEST };

  } else if (angle > 292.5 && angle <= 337.5) {
    return { NORTH, WEST };

  } else {
    return { NORTH, NONE };
  }
}

}; // namespace zen

#endif // ZEN_UTIL_HPP

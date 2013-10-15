#ifndef ZEN_ALGORITHM_HPP
#define ZEN_ALGORITHM_HPP

#include <cmath> // atan2
#include <utility> // pair

namespace zen {

namespace algorithm {

enum direction { NONE, LEFT, RIGHT, TOP, BOTTOM };

class corner {
  public:
    std::pair<direction, direction>
    operator()(const int & x, const int & y,
               const int & width, const int & height)
    {
      double norm_x = (double)(x - width / 2) / (width / 2);
      double norm_y = (double)(y - height / 2) / (height / 2);
      double angle = (180 / M_PI) * (M_PI - std::atan2(norm_x, norm_y));

      // 360 / 8 = 45; 45 / 2 = 22.5
      if (angle >  22.5 && angle <=  67.5) {
        return { TOP, RIGHT };

      } else if (angle >  67.5 && angle <= 112.5) {
        return { NONE, RIGHT };

      } else if (angle > 112.5 && angle <= 157.5) {
        return { BOTTOM, RIGHT };

      } else if (angle > 157.5 && angle <= 202.5) {
        return { BOTTOM, NONE };

      } else if (angle > 202.5 && angle <= 247.5) {
        return { BOTTOM, LEFT };

      } else if (angle > 247.5 && angle <= 292.5) {
        return { NONE, LEFT };

      } else if (angle > 292.5 && angle <= 337.5) {
        return { TOP, LEFT };

      } else {
        return { TOP, NONE };
      }

    }

}; // class corner

}; // namespace algorithm

}; // namespace zen

#endif // ZEN_ALGORITHM_HPP

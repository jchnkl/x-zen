#ifndef X_CURSOR_HPP
#define X_CURSOR_HPP

#include <unordered_map>
#include <X11/cursorfont.h>
#include "xpp/xpp.hpp"
#include "types.hpp"

namespace zen {

class cursor
  : public xpp::x::cursor<cursor, x::connection>
{
  public:
    cursor(x::connection & c)
      : m_c(c)
      , m_font(x::font::open_checked(c, "cursor"))
    {}

    const x::cursor &
    operator[](const uint16_t & source_char)
    {
      try {
        return m_cursors.at(source_char);
      } catch (...) {
        m_cursors.insert({ source_char,
                           x::cursor::create_glyph_checked(
                             m_c, m_font, m_font,
                             source_char, source_char + 1,
                             0, 0, 0, 0xffff, 0xffff, 0xffff)
                         });
        return m_cursors.at(source_char);
      }
    }

  private:
    x::connection & m_c;
    x::font m_font;
    std::unordered_map<uint16_t, x::cursor> m_cursors;
}; // class cursors

}; // namespace x

#endif // X_CURSOR_HPP

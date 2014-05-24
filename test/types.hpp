#ifndef ZEN_TYPES_HPP
#define ZEN_TYPES_HPP

#include "xpp/xpp.hpp"

namespace x {
  typedef xpp::connection<
                          // xpp::render::extension,
                          // xpp::composite::extension
                            >
                            connection;
  typedef xpp::event::registry<connection &> registry;

  typedef xpp::font<connection &> font;
  typedef xpp::cursor<connection &> cursor;
  typedef xpp::window<connection &, xpp::x::drawable> window;

  typedef xpp::x::event::create_notify<connection &> create_notify;
  typedef xpp::x::event::destroy_notify<connection &> destroy_notify;

  typedef xpp::x::event::expose<connection &> expose;
  typedef xpp::x::event::motion_notify<connection &> motion_notify;
  typedef xpp::x::event::button_press<connection &> button_press;
  typedef xpp::x::event::button_release<connection &> button_release;
  typedef xpp::x::event::property_notify<connection &> property_notify;
  typedef xpp::x::event::client_message<connection &> client_message;
  typedef xpp::x::event::map_request<connection &> map_request;
  typedef xpp::x::event::configure_request<connection &> configure_request;
}; // namespace x

namespace zen {
  typedef x::window window;
  enum direction { NORTH, SOUTH, EAST, WEST, NONE };
}; // namespace zen

#endif // ZEN_TYPES_HPP

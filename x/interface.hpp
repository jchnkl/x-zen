#ifndef X_INTERFACE_HPP
#define X_INTERFACE_HPP

#include <climits>
#include <vector>

#define MAX_PRIORITY UINT32_MAX

#define EVENT(NAMESPACE, CLASS, TYPE, STRUCT) \
namespace NAMESPACE {                                                         \
  struct CLASS {                                                              \
    STRUCT * const operator*(void) { return event; }                          \
    STRUCT * const operator->(void) { return event; }                         \
    const int type = TYPE;                                                    \
    STRUCT * event;                                                           \
  };                                                                          \
};

namespace x {

namespace interface {

namespace event {

// EVENT(key,        press,    XCB_KEY_PRESS,         xcb_key_press_event_t)
// EVENT(key,        release,  XCB_KEY_RELEASE,       xcb_key_release_event_t)
// EVENT(button,     press,    XCB_BUTTON_PRESS,      xcb_button_press_event_t)
// EVENT(button,     release,  XCB_BUTTON_RELEASE,    xcb_button_release_event_t)
// EVENT(motion,     notify,   XCB_MOTION_NOTIFY,     xcb_motion_notify_event_t)
// EVENT(enter,      notify,   XCB_ENTER_NOTIFY,      xcb_enter_notify_event_t)
// EVENT(leave,      notify,   XCB_LEAVE_NOTIFY,      xcb_leave_notify_event_t)
// EVENT(focus,      in,       XCB_FOCUS_IN,          xcb_focus_in_event_t)
// EVENT(focus,      out,      XCB_FOCUS_OUT,         xcb_focus_out_event_t)
// EVENT(keymap,     notify,   XCB_KEYMAP_NOTIFY,     xcb_keymap_notify_event_t)
// EVENT(expose,     event,    XCB_EXPOSE,            xcb_expose_event_t)
// EVENT(graphics,   exposure, XCB_GRAPHICS_EXPOSURE, xcb_graphics_exposure_event_t)
// EVENT(no,         exposure, XCB_NO_EXPOSURE,       xcb_no_exposure_event_t)
// EVENT(visibility, notify,   XCB_VISIBILITY_NOTIFY, xcb_visibility_notify_event_t)
// EVENT(create,     notify,   XCB_CREATE_NOTIFY,     xcb_create_notify_event_t)
// EVENT(destroy,    notify,   XCB_DESTROY_NOTIFY,    xcb_destroy_notify_event_t)
// EVENT(unmap,      notify,   XCB_UNMAP_NOTIFY,      xcb_unmap_notify_event_t)
// EVENT(map,        notify,   XCB_MAP_NOTIFY,        xcb_map_notify_event_t)
// EVENT(map,        request,  XCB_MAP_REQUEST,       xcb_map_request_event_t)
// EVENT(reparent,   notify,   XCB_REPARENT_NOTIFY,   xcb_reparent_notify_event_t)
// EVENT(configure,  notify,   XCB_CONFIGURE_NOTIFY,  xcb_configure_notify_event_t)
// EVENT(configure,  request,  XCB_CONFIGURE_REQUEST, xcb_configure_request_event_t)
// EVENT(gravity,    notify,   XCB_GRAVITY_NOTIFY,    xcb_gravity_notify_event_t)
// EVENT(resize,     request,  XCB_RESIZE_REQUEST,    xcb_resize_request_event_t)
// EVENT(circulate,  notify,   XCB_CIRCULATE_NOTIFY,  xcb_circulate_notify_event_t)
// EVENT(circulate,  request,  XCB_CIRCULATE_REQUEST, xcb_circulate_request_event_t)
// EVENT(property,   notify,   XCB_PROPERTY_NOTIFY,   xcb_property_notify_event_t)
// EVENT(selection,  clear,    XCB_SELECTION_CLEAR,   xcb_selection_clear_event_t)
// EVENT(selection,  request,  XCB_SELECTION_REQUEST, xcb_selection_request_event_t)
// EVENT(selection,  notify,   XCB_SELECTION_NOTIFY,  xcb_selection_notify_event_t)
// EVENT(colormap,   notify,   XCB_COLORMAP_NOTIFY,   xcb_colormap_notify_event_t)
// EVENT(client,     message,  XCB_CLIENT_MESSAGE,    xcb_client_message_event_t)
// EVENT(mapping,    notify,   XCB_MAPPING_NOTIFY,    xcb_mapping_notify_event_t)

typedef std::vector<std::pair<unsigned int, int>> priorities;

class dispatcher {
  public:
    virtual ~dispatcher(void) {}
    template<typename Handler, typename Event>
      void dispatch(Handler *, Event *);
}; // class dispatcher

template<typename Event>
class sink {
  public:
    virtual ~sink(void) {}
    virtual void handle(Event * e) = 0;
}; // class sink

class source {
  public:
    virtual ~source(void) {}
    virtual void run(void) = 0;
    virtual void attach(const priorities &, dispatcher *) = 0;
    virtual void detach(const priorities &, dispatcher *) = 0;
}; // class source

class container {
  public:
    virtual ~container(void) {}
    virtual dispatcher * const at(const unsigned int &) const = 0;
}; // class container

// O(1) event dispatcher
// container[window]->dispatch(e) ..
template<typename Event,
         int Type, int Priority,
         unsigned int (* Window)(Event * const)>
class adapter : public dispatcher
              , public sink<Event>
{
  public:
    adapter(source & source, const container & container)
      : m_source(source), m_container(container)
    {
      m_source.attach({ { Priority, Type } }, this);
    }

    ~adapter(void)
    {
      m_source.detach({ { Priority, Type } }, this);
    }

    void handle(Event * e)
    {
      auto * d = m_container.at(Window(e));
      d->dispatch(d, e);
    }

  private:
    source & m_source;
    const container & m_container;
}; // class adapter

// TODO: multi - adapter:
// template<typename ... ETC>
// class mult : public adapter<ETC> ...
// question: how to get multiple variadic template parameters?

// with event object
// Object object = container(window)
// for (handler : handlers) handler->handle(object, event)
namespace object {

template <typename Object>
class container {
  public:
    virtual ~container(void) {}
    virtual Object * const at(const unsigned int &) = 0;
};

template<typename Object, typename Event,
         int Type, int Priority,
         unsigned int (* Window)(Event * const)>
class adapter : public dispatcher
              , public sink<Event>
{
  public:
    adapter(source & source, container<Object> & container)
      : m_source(source)
      , m_container(container)
    {
      m_source.attach({ { Priority, Type } }, this);
    }

    ~adapter(void)
    {
      m_source.detach({ { Priority, Type } }, this);
    }

    void handle(Event * const e)
    {
      handle(m_container.at(Window(e)), e);
    }

    virtual void handle(Object * const, Event * const) = 0;

  private:
    source & m_source;
    container<Object> & m_container;
}; // class adapter

}; // namespace object

template<typename Handler, typename Event>
void dispatcher::dispatch(Handler * h, Event * e)
{
  try {
    dynamic_cast<sink<Event> &>(*h).handle(e);
  } catch (...) {}
}

}; // namespace event

}; // namespace interface

}; // namespace x

#endif // X_INTERFACE_HPP

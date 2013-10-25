#ifndef ZEN_CLIENT_MANAGER_HPP
#define ZEN_CLIENT_MANAGER_HPP

#include <iostream>
#include <algorithm>
#include <deque>
#include <unordered_map>

#include "../x/interface.hpp"
#include "../zen/client.hpp"
#include "../zen/client_wm_size_hints.hpp"

namespace zen {

namespace client {

namespace xevent = x::interface::event;

class manager
  : public interface::manager
  , public xevent::dispatcher
  , public xevent::sink<xcb_create_notify_event_t>
  , public xevent::sink<xcb_destroy_notify_event_t>
{
  public:
    typedef std::deque<xcb_window_t> window_deque;
    typedef std::unordered_map<xcb_window_t, client::ptr> window_client_map;

    friend std::ostream & operator<<(std::ostream &, const manager &);

    class iterator : public interface::client::iterator {
      public:
        iterator(const window_client_map & clients,
                 const window_deque::iterator & iterator)
          : m_iterator(iterator), m_clients(clients)
        {}

        std::shared_ptr<interface::client::iterator> clone(void)
        {
          return std::shared_ptr<interface::client::iterator>(
              new iterator(*this));
        }

        bool operator==(const interface::client::iterator & other)
        {
          return typeid(*this) == typeid(other)
            && equal(dynamic_cast<const iterator &>(other));
        }

        bool operator!=(const interface::client::iterator & other)
        {
          return ! this->operator==(other);
        }

        bool equal(const iterator & other)
        {
          return m_iterator == other.m_iterator;
        }

        bool operator<(const interface::client::iterator & other)
        {
          return typeid(*this) == typeid(other)
            && (dynamic_cast<const iterator &>(*this).m_iterator
                <
                dynamic_cast<const iterator &>(other).m_iterator);
        }

        bool operator>(const interface::client::iterator & other)
        {
          return typeid(*this) == typeid(other)
            && (dynamic_cast<const iterator &>(*this).m_iterator
                >
                dynamic_cast<const iterator &>(other).m_iterator);
        }

        bool operator<=(const interface::client::iterator & other)
        {
          return typeid(*this) == typeid(other)
            && (dynamic_cast<const iterator &>(*this).m_iterator
                <=
                dynamic_cast<const iterator &>(other).m_iterator);
        }

        bool operator>=(const interface::client::iterator & other)
        {
          return typeid(*this) == typeid(other)
            && (dynamic_cast<const iterator &>(*this).m_iterator
                >=
                dynamic_cast<const iterator &>(other).m_iterator);
        }

        interface::client::iterator & operator+=(const difference_type & n)
        {
          m_iterator += n;
          return *this;
        }

        interface::client::iterator & operator-=(const difference_type & n)
        {
          m_iterator -= n;
          return *this;
        }

        difference_type operator-(const interface::client::iterator & other)
        {
          if (typeid(*this) == typeid(other)) {
            return m_iterator - dynamic_cast<const iterator &>(other).m_iterator;
          } else {
            return 0;
          }
        }

        const client::ptr & operator[](const difference_type & n)
        {
          return m_clients.at(m_iterator[n]);
        }

        iterator & operator++(void)
        {
          ++m_iterator;
          return *this;
        }

        iterator & operator--(void)
        {
          --m_iterator;
          return *this;
        }

        const client::ptr & operator*(void)
        {
          return m_clients.at(*m_iterator);
        }

        interface::client & operator->(void)
        {
          return *m_clients.at(*m_iterator);
        }

      private:
        window_deque::iterator m_iterator;
        const window_client_map & m_clients;
    };

    client::ptr_iterator begin(void)
    {
      return client::ptr_iterator(
          std::shared_ptr<interface::client::iterator>(
            new iterator(m_clients, m_client_order.begin())));
    }

    client::ptr_iterator end(void)
    {
      return client::ptr_iterator(
          std::shared_ptr<interface::client::iterator>(
            new iterator(m_clients, m_client_order.end())));
    }

    manager(x::connection & c, xevent::source & s,
            interface::client::factory & client_factory)
      : m_c(c), m_s(s), m_client_factory(client_factory)
    {
      s.insert(m_priority_masks, this);
    }

    ~manager(void)
    {
      m_s.remove(m_priority_masks, this);
    }

    void
    handle(xcb_create_notify_event_t * e)
    {
      insert(e->window);
    }

    void
    handle(xcb_destroy_notify_event_t * e)
    {
      remove(e->window);
    }

    client::ptr
    operator[](const xcb_window_t & id)
    {
      try {
        return m_clients.at(id);
      } catch (...) {
        return nullptr;
      }
    }

    void
    insert(xcb_window_t window)
    {
      m_client_order.push_front(window);
      m_clients[window] = m_client_factory.make(window);
    }

    void
    remove(xcb_window_t window)
    {
      m_client_order.erase(
          std::find(m_client_order.begin(), m_client_order.end(), window));
      m_clients.erase(window);
    }

    const window_client_map &
    clients(void)
    {
      return m_clients;
    }

  private:
    x::connection & m_c;
    xevent::source & m_s;
    interface::client::factory & m_client_factory;

    window_deque m_client_order;
    window_client_map m_clients;

    priority_masks m_priority_masks =
      { { UINT_MAX, XCB_CREATE_NOTIFY }
      , { UINT_MAX, XCB_DESTROY_NOTIFY }
      };

}; // class client_manager

}; // namespace client

}; // namespace zen

#endif // ZEN_CLIENT_MANAGER_HPP

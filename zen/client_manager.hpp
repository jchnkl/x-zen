#ifndef ZEN_CLIENT_MANAGER_HPP
#define ZEN_CLIENT_MANAGER_HPP

#include <iostream>
#include <algorithm>
#include <deque>
#include <unordered_map>

#include "client.hpp"

namespace zen {

namespace client {

namespace event = x::interface::event;

class manager
  , public event::dispatcher
  , public event::sink<xcb_create_notify_event_t>
  , public event::sink<xcb_destroy_notify_event_t>
{
  public:
    typedef std::deque<xcb_window_t> window_deque;
    typedef std::unordered_map<xcb_window_t, client_ptr> window_client_map;

    friend std::ostream & operator<<(std::ostream &, const manager &);

    template<typename T>
    class iterator
      : public std::iterator<std::random_access_iterator_tag, T>
    {
      public:
        template<typename U>
        friend bool operator==(const iterator<U> &, const iterator<U> &);

        template<typename U>
        friend bool operator!=(const iterator<U> &, const iterator<U> &);

        template<typename U>
        friend bool operator<(const iterator<U> &, const iterator<U> &);

        template<typename U>
        friend bool operator>(const iterator<U> &, const iterator<U> &);

        template<typename U>
        friend bool operator<=(const iterator<U> &, const iterator<U> &);

        template<typename U>
        friend bool operator>=(const iterator<U> &, const iterator<U> &);

        iterator(window_client_map & clients, window_deque::iterator iterator)
          : m_clients(clients), m_iterator(iterator)
        {}

        iterator(const iterator & other)
          : m_clients(other.m_clients), m_iterator(other.m_iterator)
        {}

        iterator & operator=(const iterator & rhs)
        {
          m_iterator = rhs.m_iterator;
          return *this;
        }

        iterator & operator++(void) // prefix
        {
          ++m_iterator;
          return *this;
        }

        iterator & operator--(void) // prefix
        {
          --m_iterator;
          return *this;
        }

        iterator operator++(int) // postfix
        {
          iterator copy = *this;
          ++(*this);
          return copy;
        }

        iterator operator--(int) // postfix
        {
          iterator copy = *this;
          --(*this);
          return copy;
        }

        iterator operator+(const window_deque::difference_type & n)
        {
          iterator copy(*this);
          return copy += n;
        }

        iterator & operator+=(const window_deque::difference_type & n)
        {
          m_iterator += n;
          return *this;
        }

        iterator operator-(const window_deque::difference_type & n)
        {
          iterator copy(*this);
          return copy -= n;
        }

        iterator & operator-=(const window_deque::difference_type & n)
        {
          m_iterator -= n;
          return *this;
        }

        T & operator*(void) const
        {
          return *m_clients[*m_iterator];
        }

        T & operator->(void) const
        {
          return *m_clients[*m_iterator];
        }

      protected:
        window_client_map & m_clients;
        window_deque::iterator m_iterator;
    };

    manager(x::connection & c, event::source & s)
      : m_c(c), m_s(s)
    {
      s.insert(this);
    }

    ~manager(void)
    {
      m_s.remove(this);
    }

    iterator<client>
    begin(void)
    {
      return iterator<client>(m_clients, m_client_order.begin());
    }

    iterator<client>
    end(void)
    {
      return iterator<client>(m_clients, m_client_order.end());
    }

    priority_masks
    masks(void)
    {
      return { { UINT_MAX, XCB_CREATE_NOTIFY }
             , { UINT_MAX, XCB_DESTROY_NOTIFY } };
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

    void
    insert(xcb_window_t window)
    {
      m_client_order.push_front(window);
      m_clients[window] = client_ptr(new client(m_c, m_s, window));
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
    event::source & m_s;

    window_deque m_client_order;
    window_client_map m_clients;
}; // class client_manager

std::ostream & operator<<(std::ostream & os, const manager & cm)
{
  std::size_t i = 0;
  for (i = 0; i < cm.m_client_order.size(); ++i) {
    os << *cm.m_clients.at(cm.m_client_order[i]);

    if (i < cm.m_client_order.size() - 1) {
      os << ", ";
    }
  }

  return os;
}

template<typename T>
bool
operator==(const manager::iterator<T> & lhs, const manager::iterator<T> & rhs)
{
  return lhs.m_iterator == rhs.m_iterator;
}

template<typename T>
bool
operator!=(const manager::iterator<T> & lhs, const manager::iterator<T> & rhs)
{
  return ! (lhs == rhs);
}

template<typename T>
bool
operator<(const manager::iterator<T> & lhs, const manager::iterator<T> & rhs)
{
  return lhs.m_iterator < rhs.m_iterator;
}

template<typename T>
bool
operator>(const manager::iterator<T> & lhs, const manager::iterator<T> & rhs)
{
  return lhs.m_iterator > rhs.m_iterator;
}

template<typename T>
bool
operator<=(const manager::iterator<T> & lhs, const manager::iterator<T> & rhs)
{
  return lhs.m_iterator <= rhs.m_iterator;
}

template<typename T>
bool
operator>=(const manager::iterator<T> & lhs, const manager::iterator<T> & rhs)
{
  return lhs.m_iterator >= rhs.m_iterator;
}

}; // namespace client

}; // namespace zen

#endif // ZEN_CLIENT_MANAGER_HPP

#ifndef ZEN_INTERFACE_HPP
#define ZEN_INTERFACE_HPP

#include <iostream>
#include <memory>

#include "../x/window.hpp"

namespace zen {

namespace interface {

class client;

class client : public x::window {
  public:
    typedef std::shared_ptr<client> ptr;

    friend std::ostream & operator<<(std::ostream &, client &);

    client(x::connection & c, xcb_window_t w)
      : x::window(c, w)
    {}

    client(client::ptr client)
      : x::window(client->m_c, client->m_window),  m_client(client)
    {}

    virtual ~client(void) {}

    virtual int x(void)
    {
      return m_client->x();
    }

    virtual int y(void)
    {
      return m_client->y();
    }

    virtual unsigned int width(void)
    {
      return m_client->width();
    }

    virtual unsigned int height(void)
    {
      return m_client->height();
    }

    virtual unsigned int border_width(void)
    {
      return m_client->border_width();
    }

    virtual xcb_window_t sibling(void)
    {
      return m_client->sibling();
    }

    virtual xcb_stack_mode_t stack_mode(void)
    {
      return m_client->stack_mode();
    }

    virtual client & x(int x)
    {
      return m_client->x(x);
    }

    virtual client & y(int y)
    {
      return m_client->y(y);
    }

    virtual client & width(int width)
    {
      return m_client->width(width);
    }

    virtual client & height(int height)
    {
      return m_client->height(height);
    }

    virtual client & border_width(unsigned int border_width)
    {
      return m_client->border_width(border_width);
    }

    virtual client & sibling(xcb_window_t sibling)
    {
      return m_client->sibling(sibling);
    }

    virtual client & stack_mode(xcb_stack_mode_t stack_mode)
    {
      return m_client->stack_mode(stack_mode);
    }

    virtual client & configure(void)
    {
      return m_client->configure();
    }

  protected:
    client::ptr m_client;

}; // class client

class manager {
  public:
    virtual ~manager(void) {}

    class iterator
      : public std::iterator<std::random_access_iterator_tag, client::ptr> {
      public:
        virtual std::shared_ptr<iterator> clone(void) = 0;
        // i == j
        virtual bool operator==(const iterator &) = 0;
        // i != j
        virtual bool operator!=(const iterator &) = 0;

        // a < b
        virtual bool operator<(const iterator &) = 0;
        // a > b
        virtual bool operator>(const iterator &) = 0;
        // a >= b
        virtual bool operator<=(const iterator &) = 0;
        // a <= b
        virtual bool operator>=(const iterator &) = 0;

        // r += n
        virtual iterator & operator+=(const difference_type &) = 0;
        // r -= n
        virtual iterator & operator-=(const difference_type &) = 0;

        // i - n
        // virtual iterator operator-(const difference_type &) = 0;
        // b - a
        virtual difference_type operator-(const iterator &) = 0;
        // i[n]
        virtual const client::ptr & operator[](const difference_type &) = 0;

        // ++i
        virtual iterator & operator++(void) = 0;
        // --i
        virtual iterator & operator--(void) = 0;
        // i++
        // virtual iterator operator++(int) = 0;
        // i--
        // virtual iterator operator--(int) = 0;

        // *i
        virtual const client::ptr & operator*(void) = 0;
        // i->m
        virtual client & operator->(void) = 0;
    };

    class client_ptr_iterator {
      public:
        typedef client::ptr                     value_type;
        typedef iterator::difference_type       difference_type;
        typedef iterator::pointer               pointer;
        typedef iterator::reference             reference;
        typedef std::random_access_iterator_tag iterator_category;

        client_ptr_iterator(const std::shared_ptr<iterator> & iterator)
          : m_iterator(iterator)
        {}

        client_ptr_iterator(const client_ptr_iterator & other)
        {
          m_iterator = other.m_iterator->clone();
        }

        client_ptr_iterator & operator=(const client_ptr_iterator & other)
        {
          m_iterator = other.m_iterator->clone();
          return *this;
        }

        bool operator==(const client_ptr_iterator & other)
        {
          return m_iterator->operator==(*other.m_iterator);
        }

        bool operator!=(const client_ptr_iterator & other)
        {
          return m_iterator->operator!=(*other.m_iterator);
        }

        bool operator<(const client_ptr_iterator & other)
        {
          return m_iterator->operator<(*other.m_iterator);
        }

        bool operator>(const client_ptr_iterator & other)
        {
          return m_iterator->operator>(*other.m_iterator);
        }

        bool operator<=(const client_ptr_iterator & other)
        {
          return m_iterator->operator<=(*other.m_iterator);
        }

        bool operator>=(const client_ptr_iterator & other)
        {
          return m_iterator->operator>=(*other.m_iterator);
        }

        client_ptr_iterator & operator+=(const difference_type & n)
        {
          m_iterator->operator+=(n);
          return *this;
        }

        client_ptr_iterator & operator-=(const difference_type & n)
        {
          m_iterator->operator-=(n);
          return *this;
        }

        client_ptr_iterator operator-(const difference_type & n)
        {
          client_ptr_iterator copy = *this;
          copy.m_iterator->operator-=(n);
          return copy;
        }

        client_ptr_iterator operator+(const difference_type & n)
        {
          client_ptr_iterator copy = *this;
          copy.m_iterator->operator+=(n);
          return copy;
        }

        difference_type operator-(const client_ptr_iterator & other)
        {
          return m_iterator->operator-(*other.m_iterator);
        }

        const client::ptr & operator[](const difference_type & n)
        {
          return m_iterator->operator[](n);
        }

        client_ptr_iterator & operator++(void) // prefix
        {
          m_iterator->operator++();
          return *this;
        }

        client_ptr_iterator operator++(int) // postfix
        {
          client_ptr_iterator copy = *this;
          this->operator++();
          return copy;
        }

        client_ptr_iterator & operator--(void) // prefix
        {
          m_iterator->operator--();
          return *this;
        }

        client_ptr_iterator operator--(int) // postfix
        {
          client_ptr_iterator copy = *this;
          this->operator--();
          return copy;
        }

        const client::ptr & operator*(void)
        {
          return m_iterator->operator*();
        }

        client & operator->(void)
        {
          return m_iterator->operator->();
        }

      private:
        std::shared_ptr<iterator> m_iterator;
    };

    virtual client_ptr_iterator begin(void) = 0;
    virtual client_ptr_iterator end(void) = 0;

    virtual client::ptr operator[](const xcb_window_t &) = 0;
};

std::ostream &
operator<<(std::ostream & os, client & c)
{
  return os << c.id();
}

}; // namespace zen

}; // namespace interface

#endif // ZEN_INTERFACE_HPP

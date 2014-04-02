#ifndef X_VALUEPARAM_HPP
#define X_VALUEPARAM_HPP

#include <map>
#include <vector>

namespace x {

template<typename ... ARGS>
class foo {
  public:
    foo(ARGS ... args)
    {
      append(args ...);
      // m_mask |= ARGS ... args;
      for (int i = 0; i < sizeof...(ARGS); ++i) {
      }
    }

    template<typename T, typename ... TS>
    void append(T value, TS ... vs)
    {
      m_mask |= value;
    }

  private:
    uint32_t m_mask = 0;
    uint32_t m_values[(sizeof...(ARGS))];
    // std::vector<uint32_t> m_values; // (sizeof...(ARGS));
};

class valueparam {
  public:
    valueparam &
    set(const uint32_t & bit, const uint32_t & value)
    {
      m_has_changed = true;
      m_values_map[bit] = value;
      return *this;
    }

    uint32_t
    mask(void)
    {
      return m_mask;
    }

    uint32_t * const
    values(void)
    {
      if (m_has_changed) {
        m_values.clear();
      }

      for (auto & item : m_values_map) {
        m_values.push_back(item.second);
      }

      m_has_changed = false;

      return m_values.data();
    }

  private:
    bool m_has_changed = true;
    uint32_t m_mask = 0;
    std::vector<uint32_t> m_values;
    std::map<uint32_t, uint32_t> m_values_map;
};

}; // namespace x

#endif // X_VALUEPARAM_HPP

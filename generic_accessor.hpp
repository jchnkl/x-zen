#ifndef GENERIC_ACCESSOR_HPP
#define GENERIC_ACCESSOR_HPP

template<typename T, typename R>
struct access {
  static R get(T * t);
};

#define MAKE_ACCESSOR(NAME, MEMBER)                                           \
template<typename T>                                                          \
struct access<T, decltype(T::MEMBER)> {                                       \
  static decltype(T::MEMBER) get(T * t)                                       \
  {                                                                           \
    return t->MEMBER;                                                         \
  }                                                                           \
};                                                                            \
                                                                              \
template<typename T>                                                          \
decltype(T::MEMBER)                                                           \
get_##NAME(T * t)                                                             \
{                                                                             \
  return access<T, decltype(T::MEMBER)>::get(t);                              \
}

#endif // GENERIC_ACCESSOR_HPP

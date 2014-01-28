#ifndef _YAMAIL_TRAITS_IS_COMPLETE_H_
#define _YAMAIL_TRAITS_IS_COMPLETE_H_
#include <yamail/config.h>
#include <yamail/traits/namespace.h>

#include <type_traits>

YAMAIL_NS_BEGIN
YAMAIL_NS_TRAITS_BEGIN

namespace detail {

template <typename T>
struct is_complate_helper {
  template <typename U>
  static auto test (U*)  -> std::integral_constant<bool, sizeof(U)==sizeof(U)>;
  static auto test (...) -> std::false_type;

  using type = decltype (test ((T*) 0));
};

} // namespace detail

template <typename T>
struct is_complete : detail::is_complate_helper<T>::type {};


YAMAIL_NS_TRAITS_END
YAMAIL_NS_END

#endif // _YAMAIL_TRAITS_IS_COMPLETE_H_

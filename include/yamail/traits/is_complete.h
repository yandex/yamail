#ifndef _YAMAIL_TRAITS_IS_COMPLETE_H_
#define _YAMAIL_TRAITS_IS_COMPLETE_H_
#include <yamail/config.h>
#include <yamail/traits/namespace.h>

#include <yamail/compat/type_traits.h>

#if YAMAIL_CPP >= 11
# include <type_traits>
#else // boost emulation
# include <boost/typeof/typeof.hpp>
# include <boost/type_traits/integral_constant.hpp>
#endif

YAMAIL_FQNS_TRAITS_BEGIN

namespace detail {

#if YAMAIL_CPP >= 11
template <typename T>
struct is_complete_helper {
  template <typename U>
  static auto test (U*)  -> std::integral_constant<bool, sizeof(U)==sizeof(U)>;
  static auto test (...) -> std::false_type;

  using type = decltype (test ((T*) 0));
};
#else
template <typename T>
struct is_complete_helper {
  template <typename U>
  static boost::integral_constant<bool, sizeof(U)==sizeof(U)> test (U*);
  static boost::false_type test (...);

  typedef BOOST_TYPEOF (test ((T*) 0)) type;
};
#endif

} // namespace detail

template <typename T>
struct is_complete : detail::is_complete_helper<T>::type {};



YAMAIL_FQNS_TRAITS_END

#endif // _YAMAIL_TRAITS_IS_COMPLETE_H_

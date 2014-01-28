#ifndef _YAMAIL_TRAITS_ENABLE_IF_CONVERTIBLE_H_
#define _YAMAIL_TRAITS_ENABLE_IF_CONVERTIBLE_H_
#include <yamail/config.h>
#include <yamail/traits/namespace.h>

#include <yamail/traits/enable_type.h>

#if 1
#include <type_traits>
#else
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_convertible.hpp>
#endif

YAMAIL_NS_BEGIN
YAMAIL_NS_TRAITS_BEGIN

#if 1
template <typename From, typename To, typename Ret = detail::enable_type>
struct enable_if_convertible : std::enable_if<
      std::is_same<From,To>::value || std::is_convertible<From, To>::value
    , Ret
  >
{
};
#else
template <typename From, typename To, typename Ret = detail::enable_type>
struct enable_if_convertible : boost::enable_if<
      boost::mpl::or_<
          boost::is_same<From,To>
        , boost::is_convertible<From, To>
      >
    , Ret
  >::type
{
};
#endif

YAMAIL_NS_TRAITS_END
YAMAIL_NS_END

#endif // _YAMAIL_TRAITS_ENABLE_IF_CONVERTIBLE_H_

#ifndef _YAMAIL_TRAITS_ENABLE_IF_VALUE_OR_REF_H_
#define _YAMAIL_TRAITS_ENABLE_IF_VALUE_OR_REF_H_
#include <yamail/config.h>
#include <yamail/traits/namespace.h>

#include <yamail/traits/enable_type.h>

#if 0
#include <type_traits>
#else
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/remove_const.hpp>
#endif

YAMAIL_NS_BEGIN
YAMAIL_NS_TRAITS_BEGIN

#if 0
template <typename From, typename To, typename Ret = detail::enable_type>
struct enable_if_value_or_ref : std::enable_if<
    std::is_same<
        typename std::remove_const<
          typename std::remove_reference<From>::type
        >::type
      , To
    >::value
  , Ret
>
{
};

#else

template <typename From, typename To, typename Ret = detail::enable_type>
struct enable_if_value_or_ref : boost::enable_if<
    boost::is_same<
        typename boost::remove_const<
          typename boost::remove_reference<From>::type
        >::type
      , To
    >
  , Ret
>
{
};

#endif

YAMAIL_NS_TRAITS_END
YAMAIL_NS_END

#endif // _YAMAIL_TRAITS_ENABLE_IF_VALUE_OR_REF_H_

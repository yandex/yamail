#ifndef _YAMAIL_TRAITS_ENABLE_IF_VALUE_OR_REF_H_
#define _YAMAIL_TRAITS_ENABLE_IF_VALUE_OR_REF_H_
#include <yamail/config.h>
#include <yamail/traits/namespace.h>

#include <yamail/traits/enable_type.h>
#include <type_traits>

YAMAIL_NS_BEGIN
YAMAIL_NS_TRAITS_BEGIN

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

YAMAIL_NS_TRAITS_END
YAMAIL_NS_END

#endif // _YAMAIL_TRAITS_ENABLE_IF_VALUE_OR_REF_H_

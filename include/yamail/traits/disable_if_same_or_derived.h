#ifndef _YAMAIL_TRAITS_DISABLE_IF_SAME_OR_DERIVED_H_
#define _YAMAIL_TRAITS_DISABLE_IF_SAME_OR_DERIVED_H_
#include <yamail/config.h>
#include <yamail/traits/enable_type.h>
#include <yamail/traits/namespace.h>

#include <type_traits>

YAMAIL_NS_BEGIN
YAMAIL_NS_TRAITS_BEGIN

template <typename From, typename To, typename Ret = detail::enable_type>
struct disable_if_same_or_derived : std::enable_if<
      ! std::is_base_of<
            From
          , typename std::remove_reference<To>::type
        >::value
      , Ret
  >
{
};

YAMAIL_NS_TRAITS_END
YAMAIL_NS_END

#endif // _YAMAIL_TRAITS_DISABLE_IF_SAME_OR_DERIVED_H_

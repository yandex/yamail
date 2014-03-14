#ifndef _YAMAIL_TRAITS_DISABLE_IF_SAME_OR_DERIVED_H_
#define _YAMAIL_TRAITS_DISABLE_IF_SAME_OR_DERIVED_H_
#include <yamail/config.h>
#include <yamail/traits/enable_type.h>
#include <yamail/traits/namespace.h>

#if 0
#include <type_traits>
#else
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_base_of.hpp>
#endif

YAMAIL_NS_BEGIN
YAMAIL_NS_TRAITS_BEGIN

#if 0

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

template <typename From, typename To, typename Ret = detail::enable_type>
struct anable_if_same_or_derived : std::enable_if<
        std::is_base_of<
            From
          , typename std::remove_reference<To>::type
        >::value
      , Ret
  >
{
};

#else

template <typename From, typename To, typename Ret = detail::enable_type>
struct enable_if_same_or_derived : boost::enable_if<
        boost::is_base_of<
            From
          , typename boost::remove_reference<To>::type
        >
        , Ret
  >
{
};

template <typename From, typename To, typename Ret = detail::enable_type>
struct disable_if_same_or_derived : boost::disable_if<
        boost::is_base_of<
            From
          , typename boost::remove_reference<To>::type
        >
        , Ret
  >
{
};

#endif

YAMAIL_NS_TRAITS_END
YAMAIL_NS_END

#endif // _YAMAIL_TRAITS_DISABLE_IF_SAME_OR_DERIVED_H_

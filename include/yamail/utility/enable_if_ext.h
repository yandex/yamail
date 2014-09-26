#ifndef _YAMAIL_UTILITY_ENABLE_IF_EXT_H_
#define _YAMAIL_UTILITY_ENABLE_IF_EXT_H_
// http://stackoverflow.com/questions/24687026/implementing-variadic-type-traits
#include <yamail/config.h>

#if YAMAIL_CPP < 11

# error This file requires compiler and library support for the \
ISO C++ 2011 standard. This support is currently experimental, and must be \
enabled with the -std=c++11 or -std=gnu++11 compiler options.

#else

#include <yamail/utility/namespace.h>
#include <type_traits>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace utility {
#else
YAMAIL_FQNS_UTILITY_BEGIN
#endif // GENERATING_DOCUMENTATION

#if YAMAIL_CPP >= 14
// Note: std::integer_sequence is C++14,
// but it's easy to use your own version (even stripped down)
// for the following purpose:
template< bool... Bs >
using bool_sequence = std::integer_sequence< bool, Bs... >;
#else
// Alternatively, not using C++14:
template< bool... > struct bool_sequence {};
#endif

template< bool... Bs >
using bool_and = std::is_same< bool_sequence< Bs... >,
                               bool_sequence< ( Bs || true )... > >;

template< bool... Bs >
using bool_or = std::integral_constant< bool, !bool_and< !Bs... >::value >;

template< typename R, bool... Bs > // note: R first, no default :(
using enable_if_any = std::enable_if< bool_or< Bs... >::value, R >;

template< typename R, bool... Bs > // note: R first, no default :(
using enable_if_all = std::enable_if< bool_and< Bs... >::value, R >;

template< typename T, typename... Ts >
using are_same = bool_and< std::is_same< T, Ts >::value... >;

#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_UTILITY_END
#endif // GENERATING_DOCUMENTATION

#endif // C++11

#endif // _YAMAIL_UTILITY_ENABLE_IF_EXT_H_

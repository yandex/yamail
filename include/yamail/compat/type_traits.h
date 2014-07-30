#ifndef _YAMAIL_COMPAT_TYPE_TRAITS_H_
#define _YAMAIL_COMPAT_TYPE_TRAITS_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

#if defined(HAVE_STD_TYPE_TRAITS) && HAVE_STD_TYPE_TRAITS
# include <type_traits>
#else 
# include <boost/utility/result_of.hpp>
# include <boost/type_traits.hpp>
#endif // HAVE_STD_TYPE_TRAITS

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace compat {
#else
YAMAIL_FQNS_COMPAT_BEGIN
#endif // GENERATING_DOCUMENTATION

#if defined(HAVE_STD_TYPE_TRAITS) && HAVE_STD_TYPE_TRAITS
using std::result_of;
namespace type_traits = std;
#else
using boost::result_of;
namespace type_traits = boost;
#endif

#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_COMPAT_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_COMPAT_TYPE_TRAITS_H_

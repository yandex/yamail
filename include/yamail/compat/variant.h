#ifndef _YAMAIL_COMPAT_VARIANT_H_
#define _YAMAIL_COMPAT_VARIANT_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

// movable version of variant

#if YAMAIL_CPP >= 11
# include <yamail/utility/variant.h>
#else
# include <boost/variant.hpp>
#endif

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace compat {
#else
YAMAIL_FQNS_COMPAT_BEGIN
#endif // GENERATING_DOCUMENTATION

#if YAMAIL_CPP >= 11
 using YAMAIL_FQNS_UTILITY::variant;
 using YAMAIL_FQNS_UTILITY::get;
 using YAMAIL_FQNS_UTILITY::cget;
 using YAMAIL_FQNS_UTILITY::type_index;
#else
 using ::boost::variant;
 using ::boost::get;
 using ::boost::cget;
 using ::boost::type_index;
#endif

#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_COMPAT_END 
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_COMPAT_VARIANT_H_

#ifndef _YAMAIL_COMPAT_TYPE_TRAITS_H_
#define _YAMAIL_COMPAT_TYPE_TRAITS_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

#if defined(HAVE_STD_TYPE_TRAITS) && HAVE_STD_TYPE_TRAITS
# include <type_traits>
#else 
# include <boost/utility/result_of.hpp>
#endif // HAVE_STD_TYPE_TRAITS

YAMAIL_FQNS_COMPAT_BEGIN

#if defined(HAVE_STD_TYPE_TRAITS) && HAVE_STD_TYPE_TRAITS
using std::result_of;
#else
using boost::result_of;
#endif

YAMAIL_FQNS_COMPAT_END
#endif // _YAMAIL_COMPAT_TYPE_TRAITS_H_

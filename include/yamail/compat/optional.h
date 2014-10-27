#ifndef _YAMAIL_COMPAT_OPTIONAL_H_
#define _YAMAIL_COMPAT_OPTIONAL_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

#if defined(HAVE_STD_OPTIONAL) && HAVE_STD_OPTIONAL
# include <experimental/optional>
#else 
# include <boost/optional.hpp>
#endif // HAVE_STD_OPTIONAL

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace compat {
#else
YAMAIL_FQNS_COMPAT_BEGIN
#endif // GENERATING_DOCUMENTATION

#if defined(HAVE_STD_OPTIONAL) && HAVE_STD_OPTIONAL
using std::experimental::optional;
#else
using boost::optional;
#endif

#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_COMPAT_END 
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_COMPAT_OPTIONAL_H_

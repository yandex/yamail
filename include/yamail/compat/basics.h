#ifndef _YAMAIL_COMPAT_BASICS_H_
#define _YAMAIL_COMPAT_BASICS_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>
/*
 * function.h
 *
 *  [std|boost]::function compatibility wrapper header
 */

#if defined(HAVE_STD_ADDRESSOF) && HAVE_STD_ADDRESSOF
# include <memory>
#else
#include <boost/utility.hpp>
#endif // __HAVE_STD_ADDRESSOF

#if defined(HAVE_STD_RATIO) && HAVE_STD_RATIO
# include <ratio>
#else
#include <boost/ratio.hpp>
#endif // __HAVE_STD_ADDRESSOF

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace compat {
#else
YAMAIL_FQNS_COMPAT_BEGIN
#endif // GENERATING_DOCUMENTATION

#if defined(HAVE_STD_ADDRESSOF) && HAVE_STD_ADDRESSOF
using std::addressof;
#else
using boost::addressof;
#endif // __HAVE_STD_FUNCTION

#if defined(HAVE_STD_RATIO) && HAVE_STD_RATIO
using std::ratio;
#else
using boost::ratio;
#endif // __HAVE_STD_FUNCTION

#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_COMPAT_END
#endif // GENERATING_DOCUMENTATION

#endif /* _YAMAIL_COMPAT_BASICS_H_ */

/*
 * bind.h
 *
 *  [std|boost]::bind compatibility wrapper header
 */

#ifndef _YAMAIL_COMPAT_BIND_H_
#define _YAMAIL_COMPAT_BIND_H_

#include <yamail/config.h>
#include <yamail/compat/namespace.h>

#if defined(__HAVE_STD_BIND)
# include <functional>
#else
# include <boost/bind.hpp>
#endif // __HAVE_STD_BIND

YAMAIL_FQNS_COMPAT_BEGIN

#if defined(__HAVE_STD_BIND)
using std::bind;
#else
using boost::bind;
#endif // __HAVE_STD_BIND

YAMAIL_FQNS_COMPAT_END

#endif /* _YAMAIL_COMPAT_BIND_H_ */

#ifndef _YAMAIL_COMPAT_MOVE_H_
#define _YAMAIL_COMPAT_MOVE_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

#if defined(HAVE_STD_MOVE) && HAVE_STD_MOVE
# include <utility>
#else
# include <boost/move/move.hpp>
#endif // HAVE_STD_MOVE

YAMAIL_FQNS_COMPAT_BEGIN

#if defined(HAVE_STD_MOVE) && HAVE_STD_MOVE
using std::move;
#else
using boost::move;
#endif // HAVE_STD_MOVE

YAMAIL_FQNS_COMPAT_END

#endif // _YAMAIL_COMPAT_MOVE_H_

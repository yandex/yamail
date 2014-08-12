#ifndef _YAMAIL_COMPAT_STATIC_ASSERT_H_
#define _YAMAIL_COMPAT_STATIC_ASSERT_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>

#if !defined(HAVE_STATIC_ASSERT) || !HAVE_STATIC_ASSERT
# include <boost/static_assert.hpp>

# define static_assert BOOST_STATIC_ASSERT
#endif

#endif // _YAMAIL_COMPAT_STATIC_ASSERT_H_

#ifndef _YAMAIL_CONFIG_H_
#define _YAMAIL_CONFIG_H_

#if !defined(BOOST_SPIRIT_USE_PHOENIX_V3)
# define BOOST_SPIRIT_USE_PHOENIX_V3
#endif

// C++03 / C++11 compability layer
#include <yamail/cplusplus.h>

#if HAVE_VISIBILITY
# define API_PUBLIC __attribute__ ((visibility("default")))
# define API_PRIVATE __attribute__ ((visibility("hidden")))
#else
# define API_PUBLIC
# define API_PRIVATE
#endif

#endif // _YAMAIL_CONFIG_H_

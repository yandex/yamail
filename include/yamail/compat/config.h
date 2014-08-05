#ifndef _YAMAIL_COMPAT_CONFIG_H_
#define _YAMAIL_COMPAT_CONFIG_H_
#include <yamail/config.h>

// define YAMAIL_COMPAT_USE_BOOST to use boost libraries everywhere

#if __cplusplus >= 201300L
# define YAMAIL_COMPAT_CPP_VERSION 201400L
#elif __cplusplus >= 201103L
# define YAMAIL_COMPAT_CPP_VERSION 201100L
#else
# define YAMAIL_COMPAT_CPP_VERSION 200300L
#endif

#if !defined(YAMAIL_COMPAT_USE_BOOST) && YAMAIL_COMPAT_CPP_VERSION >= 201400L
// C++ 14
# if !defined(HAVE_STD_MAKE_UNIQUE)
#  define HAVE_STD_MAKE_UNIQUE 1
# endif
#else
# if !defined(HAVE_STD_MAKE_UNIQUE)
#  define HAVE_STD_MAKE_UNIQUE 0
# endif
#endif

#if !defined(YAMAIL_COMPAT_USE_BOOST) && YAMAIL_COMPAT_CPP_VERSION >= 201100L
// C++ 11
# if !defined(HAVE_STD_BIND)
#  define HAVE_STD_BIND 1
# endif

# if !defined(HAVE_STD_CHRONO)
#  define HAVE_STD_CHRONO 1
# endif

# if !defined(HAVE_STD_EXCEPTION)
#  define HAVE_STD_EXCEPTION 1
# endif

# if !defined(HAVE_STD_FUNCTION)
#  define HAVE_STD_FUNCTION 1
# endif

# if !defined(HAVE_STD_MOVE)
#  define HAVE_STD_MOVE 1
# endif

# if !defined(HAVE_STD_SHARED_PTR)
#  define HAVE_STD_SHARED_PTR 1
# endif

# if !defined(HAVE_STD_SYSTEM)
#  define HAVE_STD_SYSTEM 1
# endif

# if !defined(HAVE_STD_THREAD)
#  define HAVE_STD_THREAD 1
# endif

# if !defined(HAVE_STD_UNORDERED)
#  define HAVE_STD_UNORDERED 1
# endif

# if !defined(HAVE_STD_TYPE_TRAITS)
#  define HAVE_STD_TYPE_TRAITS 1
# endif

# if !defined(HAVE_STD_ADDRESSOF)
#  define HAVE_STD_ADDRESSOF 1
# endif

#else
// C++ 03
# if !defined(HAVE_STD_BIND)
#  define HAVE_STD_BIND 0
# endif

# if !defined(HAVE_STD_CHRONO)
#  define HAVE_STD_CHRONO 0
# endif

# if !defined(HAVE_STD_EXCEPTION)
#  define HAVE_STD_EXCEPTION 0
# endif

# if !defined(HAVE_STD_FUNCTION)
#  define HAVE_STD_FUNCTION 0
# endif

# if !defined(HAVE_STD_MOVE)
#  define HAVE_STD_MOVE 0
# endif

# if !defined(HAVE_STD_SHARED_PTR)
#  define HAVE_STD_SHARED_PTR 0
# endif

# if !defined(HAVE_STD_SYSTEM)
#  define HAVE_STD_SYSTEM 0
# endif

# if !defined(HAVE_STD_THREAD)
#  define HAVE_STD_THREAD 0
#  define BOOST_THREAD_PROVIDES_FUTURE 1
# endif

# if !defined(HAVE_STD_UNORDERED)
#  define HAVE_STD_UNORDERED 0
# endif

# if !defined(HAVE_STD_TYPE_TRAITS)
#  define HAVE_STD_TYPE_TRAITS 0
# endif

# if !defined(HAVE_STD_ADDRESSOF)
#  define HAVE_STD_ADDRESSOF 0
# endif

#endif // C++ 11

#endif // _YAMAIL_COMPAT_CONFIG_H_

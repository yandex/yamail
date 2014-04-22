#ifndef _YAMAIL_COMPAT_CONFIG_H_
#define _YAMAIL_COMPAT_CONFIG_H_
#include <yamail/config.h>

#if _cplusplus >= 201103L
// C++ 14
# if !defined(HAVE_STD_MAKE_UNIQUE)
#  define HAVE_STD_MAKE_UNIQUE 1
# endif
#else
# if !defined(HAVE_STD_MAKE_UNIQUE)
#  define HAVE_STD_MAKE_UNIQUE 0
# endif
#endif

#if _cplusplus >= 201103L
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

# if !defined(HAVE_STD_SHARED_PTR)
#  define HAVE_STD_SHARED_PTR 0
# endif

# if !defined(HAVE_STD_SYSTEM)
#  define HAVE_STD_SYSTEM 0
# endif

# if !defined(HAVE_STD_THREAD)
#  define HAVE_STD_THREAD 0
# endif

# if !defined(HAVE_STD_UNORDERED)
#  define HAVE_STD_UNORDERED 0
# endif

#endif // C++ 11

#endif // _YAMAIL_COMPAT_CONFIG_H_

#ifndef _YAMAIL_CPLUSPLUS_H_
#define _YAMAIL_CPLUSPLUS_H_
#include <yamail/config.h>

#if _cplusplus >= 201300L // C++1y
# define YAMAIL_CPP 14
# define YAMAIL_CPP14 __cplusplus
#elif __cplusplus >= 201103L 
# define YAMAIL_CPP 11
# define YAMAIL_CPP11 __cplusplus
#else
# define YAMAIL_CPP 3
# define YAMAIL_CPP03 __cplusplus
#endif

// C++11 or later
#if YAMAIL_CPP >= 11

# define _noexcept noexcept
# define _constexpr constexpr

# if !defined(YAMAIL_USE_RVALUES)
#  define YAMAIL_USE_RVALUES 1
# endif

#else // earlier than C++11

# define _noexcept throw()
# define _constexpr

# if !defined(YAMAIL_USE_RVALUES)
#  define YAMAIL_USE_RVALUES 0
# endif


#endif // earlier than C++11

// fix spirit (boost 1.55)
#if YAMAIL_CPP >= 11
#define BOOST_RESULT_OF_USE_TR1_WITH_DECLTYPE_FALLBACK 1
#endif

#endif // _YAMAIL_CPLUSPLUS_H_

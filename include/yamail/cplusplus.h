#ifndef _YAMAIL_CPLUSPLUS_H_
#define _YAMAIL_CPLUSPLUS_H_
#include <yamail/config.h>

#if __cplusplus >= 201103L 
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

#else // earlier than C++11

# define _noexcept throw()
# define _constexpr

#endif // earlier than C++11

// fix spirit (boost 1.55)
#if YAMAIL_CPP >= 11
#define BOOST_RESULT_OF_USE_TR1_WITH_DECLTYPE_FALLBACK 1
#endif

#endif // _YAMAIL_CPLUSPLUS_H_

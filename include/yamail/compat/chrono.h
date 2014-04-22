#ifndef _YAMAIL_COMPAT_CHRONO_H_
#define _YAMAIL_COMPAT_CHRONO_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

#if defined(HAVE_STD_CHRONO) && HAVE_STD_CHRONO
# include <chrono>
#else 
# include <boost/chrono.hpp>
#endif // HAVE_STD_CHRONO

YAMAIL_FQNS_COMPAT_BEGIN

#if defined(HAVE_STD_CHRONO) && HAVE_STD_CHRONO
namespace chrono = std::chrono;
#else
namespace chrono = boost::chrono;
#endif

YAMAIL_FQNS_COMPAT_END 

#endif // _YAMAIL_COMPAT_CHRONO_H_

#ifndef _YAMAIL_COMPAT_CHRONO_H_
#define _YAMAIL_COMPAT_CHRONO_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>
#include <yamail/compat/basics.h> // for compat::ratio

#if defined(HAVE_STD_CHRONO) && HAVE_STD_CHRONO
# include <chrono>
# include <yamail/compat/chrono_io.h>
#else 
#include <boost/chrono.hpp>
#include <boost/ratio.hpp>
#if 0
#include <boost/chrono/chrono.hpp>
#include <boost/chrono/process_cpu_clocks.hpp>
#include <boost/chrono/thread_clock.hpp>
#include <boost/chrono/ceil.hpp>
#include <boost/chrono/floor.hpp>
#include <boost/chrono/round.hpp>
#include <boost/chrono/io/timezone.hpp>
#include <boost/chrono/io/time_point_io.hpp>
#endif
#endif // HAVE_STD_CHRONO

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace compat {
#else
YAMAIL_FQNS_COMPAT_BEGIN
#endif // GENERATING_DOCUMENTATION

#if defined(HAVE_STD_CHRONO) && HAVE_STD_CHRONO
namespace chrono = std::chrono;
#else
namespace chrono = boost::chrono;
#endif

#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_COMPAT_END 
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_COMPAT_CHRONO_H_

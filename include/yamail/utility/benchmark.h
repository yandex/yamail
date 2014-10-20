#ifndef _YAMAIL_UTILITY_BENCHMARK_H_
#define _YAMAIL_UTILITY_BENCHMARK_H_
#include <yamail/config.h>
#include <yamail/utility/namespace.h>
#include <yamail/compat/chrono.h>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace utility {
#else
YAMAIL_FQNS_UTILITY_BEGIN
#endif // GENERATING_DOCUMENTATION

#define start_benchmark() \
  const YAMAIL_FQNS_COMPAT::chrono::high_resolution_clock::time_point \
      __bench__ = YAMAIL_FQNS_COMPAT::chrono::high_resolution_clock::now ();

#define end_benchmark() \
	(YAMAIL_FQNS_COMPAT::chrono::high_resolution_clock::now () - __bench__)

#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_UTILITY_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_UTILITY_BENCHMARK_H_

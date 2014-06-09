#ifndef _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_FLAGS_H_
#define _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_FLAGS_H_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/namespace.h>

YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN
namespace detail {

enum flag_t
{
	flag_interruption_blocked = 1 << 0,
	flag_interruption_requested = 1 << 1,
	flag_thread_affinity = 1 << 2,
	flag_detached = 1 << 3
};

} // namespace detail
YAMAIL_FQNS_CONCURRENCY_COROUTINE_END
#endif // _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_FLAGS_H_

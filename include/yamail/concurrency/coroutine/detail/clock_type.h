#ifndef _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_CLOCK_TYPE_H_
#define _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_CLOCK_TYPE_H_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/namespace.h>
#include <yamail/compat/chrono.h>

YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN
namespace detail {

typedef compat::chrono::steady_clock clock_type;

} // namespace detail
YAMAIL_FQNS_CONCURRENCY_COROUTINE_END
#endif // _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_CLOCK_TYPE_H_

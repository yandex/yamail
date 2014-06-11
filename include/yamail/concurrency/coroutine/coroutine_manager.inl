#ifndef _YAMAIL_CONCURRENCY_COROUTINE_COROUTINE_MANAGER_INL_
#define _YAMAIL_CONCURRENCY_COROUTINE_COROUTINE_MANAGER_INL_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/coroutine_manager.h>

YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN

inline
coroutine_manager::coroutine_manager () BOOST_NOEXCEPT
  : def_algo_ (new round_robin ())
  , sched_algo_ (def_algo_.get ())
  , wqueue_ ()
  , wait_interval_ (compat::chrono::milliseconds (10))
  , active_coroutine_ (0)
{
}

YAMAIL_FQNS_CONCURRENCY_COROUTINE_END
#endif // _YAMAIL_CONCURRENCY_COROUTINE_COROUTINE_MANAGER_INL_

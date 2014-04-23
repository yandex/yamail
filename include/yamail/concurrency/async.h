#ifndef _YAMAIL_CONCURRENCY_ASYNC_H_
#define _YAMAIL_CONCURRENCY_ASYNC_H_
#include <yamail/config.h>
#include <yamail/concurrency/namespace.h>

#include <yamail/compat/type_traits.h>
#include <yamail/compat/thread.h>
#include <yamail/concurrency/future/future.h>

YAMAIL_FQNS_CONCURRENCY_BEGIN

enum launch {
	none = 0,
	async = 1,
	deferred = 2,
	any = async | deferred
};

enum future_status {
	ready,
	timeout,
	deferred
};

template <class Fn, class... Args>
future<typename compat::result_of<Fn (Args...)>::type>
async (Fn&& fn, Args&& ...args)
{
}

template <class Fn, class... Args>
future<typename compat::result_of<Fn (Args...)>::type>
async (launch policy, Fn&& fn, Args&& ...args)
{
}

YAMAIL_FQNS_CONCURRENCY_END

#endif // _YAMAIL_CONCURRENCY_ASYNC_H_

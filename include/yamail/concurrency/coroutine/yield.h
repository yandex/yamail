#ifndef _YAMAIL_CONCURRENCY_COROUTINE_YIELD_H_
#define _YAMAIL_CONCURRENCY_COROUTINE_YIELD_H_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/namespace.h>

#include <yamail/concurrency/coroutine/error_code.h>

YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN

class yield_t
{
  yield_t (error_code& ec) : ec_ (&ec) {}

public:
	BOOST_CONSTEXPR yield_t ()
	  : ec_ (0)
	{}

	yield_t operator[] (error_code& ec) const
	{
		return yield_t (ec);
  }

// private: // XXX

  error_code* ec_;
};

BOOST_CONSTEXPR_OR_CONST yield_t yield;

YAMAIL_FQNS_CONCURRENCY_COROUTINE_END

#include <yamail/concurrency/coroutine/detail/yield.h>

#endif // _YAMAIL_CONCURRENCY_COROUTINE_YIELD_H_

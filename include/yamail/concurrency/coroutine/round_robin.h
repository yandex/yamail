#ifndef _YAMAIL_CONCURRENCY_COROUTINE_ROUND_ROBIN_H_
#define _YAMAIL_CONCURRENCY_COROUTINE_ROUND_ROBIN_H_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/namespace.h>

#include <yamail/concurrency/coroutine/coroutine_manager.h>
#include <yamail/concurrency/coroutine/detail/fifo.h>

#include <boost/assert.hpp>


YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN

class round_robin: public sched_algorithm
{
	typedef detail::fifo rqueue_t;
	rqueue_t rqueue_;

public:
  virtual void awakened (detail::worker_coroutine* c)
  {
  	rqueue_.push (c);
  }

  virtual detail::worker_coroutine* pick_next ()
  {
  	detail::worker_coroutine* victim = 0;
  	if (! rqueue_.empty ())
  		victim = rqueue_.pop ();

  	return victim;
  }

  virtual void priority (detail::worker_coroutine* c, int prio) BOOST_NOEXCEPT
  {
  	BOOST_ASSERT (c);

  	// set only priority fo coroutine
  	// round-robin does not respect priorities.
  	c->priority (prio);
  }
};

YAMAIL_FQNS_CONCURRENCY_COROUTINE_END
#endif // _YAMAIL_CONCURRENCY_COROUTINE_ROUND_ROBIN_H_

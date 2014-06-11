#ifndef _YAMAIL_CONCURRENCY_COROUTINE_COROUTINE_H_
#define _YAMAIL_CONCURRENCY_COROUTINE_COROUTINE_H_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/namespace.h>

#include <yamail/concurrency/spinlock.h>
#include <yamail/concurrency/coroutine/clock_type.h>
#include <yamail/concurrency/coroutine/detail/worker_coroutine.h>
#include <yamail/concurrency/coroutine/detail/waiting_queue.h>

#include <yamail/compat/chrono.h>
#include <yamail/compat/mutex.h>
#include <yamail/compat/thread.h>

#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>

YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN

struct sched_algorithm
{
	virtual ~sched_algorithm () {}
	virtual void awakened (detail::worker_coroutine*) = 0;
	virtual detail::worker_coroutine* pick_next () = 0;
	virtual void priority (detail::worker_coroutine*, int) BOOST_NOEXCEPT = 0;
};

class coroutine_manager : boost::noncopyable
{
	static bool fetch_ready (detail::worker_coroutine* c)
	{
		BOOST_ASSERT (! c->is_running ());
		BOOST_ASSERT (! c->is_terminated ());

		// set coroutine to state_ready if dead-line was reached
		// set coroutine to state_ready if interruption was requested
		if (c->time_point () <= clock_type::now () || c->interruption_requested ())
			c->set_ready ();

		return c->is_ready ();
  }

public:
	coroutine_manager () BOOST_NOEXCEPT;
#if 0
	  : def_algo_ (new round_robin ())
	  , sched_algo_ (def_algo_.get ())
	  , wqueue_ ()
	  , wait_interval_ (compat::chrono::milliseconds (10))
	  , active_coroutine_ (0)
	{
  }
#endif

	virtual ~coroutine_manager () BOOST_NOEXCEPT
	{
    // coroutines will be destroyed (stack-unwinding)
    // if last reference goes out-of-scope
    // therefore destructing wqueue_ && rqueue_
    // will destroy the coroutines in this scheduler
    // if not referenced on other places

    //active_coroutine_->set_terminated();

    while ( ! wqueue_.empty() )
      run();
  }

	void set_sched_algo (sched_algorithm* algo) 
	{
		sched_algo_ = algo;
		def_algo_.reset ();
  }

  void spawn (detail::worker_coroutine* c)
  {
  	BOOST_ASSERT (c);
  	BOOST_ASSERT (c->is_ready ());

  	sched_algo_->awakened (c);
  }

  void priority (detail::worker_coroutine* c, int prio) BOOST_NOEXCEPT
  {
  	sched_algo_->priority (c, prio);
  }

  template <typename Rep, typename Period>
  void wait_interval (
    compat::chrono::duration<Rep, Period> const& wait_interval) 
  {
  	wait_interval_ = wait_interval;
  }

  template <typename Rep, typename Period>
  compat::chrono::duration<Rep, Period> wait_interval () const // BOOST_NOEXCEPT
  {
  	return wait_interval_;
  }

  void join (detail::worker_coroutine* c)
  {
  	BOOST_ASSERT (c);
  	BOOST_ASSERT (c != active_coroutine_);

  	if (active_coroutine_)
    {
    	// set active coroutine to state_waiting
      active_coroutine_->set_waiting ();

      // push active coroutine to wqueue_
      wqueue_.push (active_coroutine_);

      // add active coroutine to join list of c
      if (! c->join (active_coroutine_))
        // c must be already terminated therefore we set
        // active coroutine to state_ready 
        // FIXME: metter state_running and no suspend
        active_coroutine_->set_ready ();

      // run next coroutine
      run ();
    }
    else
    {
    	while (! c->is_terminated ())
    		// yield this thread if scheduled did not
    		// resumed some coroutines in the previous round.
    		run ();
    }

    BOOST_ASSERT (c->is_terminated ());
  }

  detail::worker_coroutine* active () BOOST_NOEXCEPT
  {
  	return active_coroutine_; 
  }

  virtual void run ()
  {
  	for (;;)
    {
    	// move all coroutines which are ready (state_ready)
    	// from waiting queue to the runnable queue.
    	wqueue_.move_to (sched_algo_, fetch_ready);
    
      // pop new coroutiune from ready queue which is not complete
      // (e.g.: coroutine in ready queue could be canceled by active coroutine.
      detail::worker_coroutine* c (sched_algo_->pick_next ());

      if (c)
      {
      	BOOST_ASSERT_MSG (c->is_ready (), 
      	  "coroutine with invalid state in ready-queue");
      	resume_ (c);
      	return;
      }
      else
      {
      	if (active_coroutine_)
      		active_coroutine_->suspend ();
      	else
        {
        	// no coroutines ready to run; the thread should sleep
        	// until earliest coroutine is scheduled to run
        	clock_type::time_point wakeup (next_wakeup ());
        	compat::this_thread::sleep_until (wakeup);
        }
        return;
      }
    }
  }


  void wait (compat::unique_lock<spinlock>& lk)
  {
  	wait_until (clock_type::time_point ((clock_type::duration::max) ()), lk);
  }

  template <typename Clock, typename Duration>
  bool wait_until (compat::chrono::time_point<Clock,Duration> const& timep,
      compat::unique_lock<spinlock>& lk)
  {
  	clock_type::time_point start (clock_type::now ());

  	BOOST_ASSERT (active_coroutine_);
  	BOOST_ASSERT (active_coroutine_->is_running ());

    // set active coroutine to state_waiting
    active_coroutine_->set_waiting ();
    // release lock
    lk.unlock ();

    // push active coroutine to wqueue_
    active_coroutine_->time_point (timep);
    wqueue_.push (active_coroutine_);

    // run next coroutine.
    run ();

    return clock_type::now () < timep;
  }

  template <typename Rep, typename Period>
  bool wait_for (compat::chrono::duration<Rep,Period> const& timeout_dur,
      compat::unique_lock<spinlock>& lk)
  {
  	return wait_until (clock_type::now() + timeout_dur, lk);
  }

  void yield ()
  {
  	BOOST_ASSERT (active_coroutine_);
  	BOOST_ASSERT (active_coroutine_->is_running ());

  	// set active coroutine to state_waiting
  	active_coroutine_->set_ready ();

  	// push active coroutine to scheduler algo
  	sched_algo_->awakened (active_coroutine_);

  	// run next coroutine.
  	run ();
  }

  clock_type::time_point next_wakeup ()
  {
  	if (wqueue_.empty ())
  		return clock_type::now () + wait_interval_;

  	clock_type::time_point wakeup (wqueue_.top ()->time_point ());
  	if ((clock_type::time_point::max) () == wakeup)
  		return clock_type::now () + wait_interval_;

  	return wakeup;
  }

  void migrate (detail::worker_coroutine* c)
  {
  	BOOST_ASSERT (c);
  	BOOST_ASSERT (c->is_ready ());

  	spawn (c);
  	run ();
  }

protected:
  typedef detail::waiting_queue wqueue_t;

  std::unique_ptr<sched_algorithm> def_algo_;
  sched_algorithm*                 sched_algo_;
  wqueue_t                         wqueue_;

  void resume_ (detail::worker_coroutine* c)
  {
    BOOST_ASSERT (c);
    BOOST_ASSERT (c->is_ready ());

    // store active coroutine in local var
    detail::worker_coroutine* tmp (active_coroutine_);

    // assign new coroutine to active
    active_coroutine_ = c;

    // set active coroutine to state_running
    active_coroutine_->set_running ();

    // check if active coroutine calls itself
    // it might happen if coroutine calls yield () and no other
    // coroutine is in ready state.
    if (tmp != active_coroutine_)
    {
    	// resume active coroutine == start or yield to
    	active_coroutine_->resume (tmp);

    	if (active_coroutine_->detached () && active_coroutine_->is_terminated ())
        active_coroutine_->deallocate ();

      // reset active coroutine to previous.
      active_coroutine_ = tmp;
    }
  }

private:
  clock_type::duration      wait_interval_;
  detail::worker_coroutine* active_coroutine_;
};

YAMAIL_FQNS_CONCURRENCY_COROUTINE_END

#include <yamail/concurrency/coroutine/round_robin.h>
#include <yamail/concurrency/coroutine/coroutine_manager.inl>

#endif // _YAMAIL_CONCURRENCY_COROUTINE_COROUTINE_H_

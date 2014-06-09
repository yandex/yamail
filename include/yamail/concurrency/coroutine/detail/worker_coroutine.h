#ifndef _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_WORKER_COROUTINE_H_
#define _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_WORKER_COROUTINE_H_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/namespace.h>

#include <yamail/compat/thread.h>
#include <yamail/compat/mutex.h>
#include <yamail/compat/exception.h>
#include <yamail/compat/move.h>

#include <map>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include <boost/coroutine/coroutine.hpp>

#include <yamail/concurrency/coroutine/detail/coroutine_base.h>
#include <yamail/concurrency/coroutine/detail/flags.h>
#include <yamail/concurrency/coroutine/detail/clock_type.h>
#include <yamail/concurrency/coroutine/detail/css.h>

#include <yamail/concurrency/spinlock.h>

YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN
namespace detail {

namespace coro = boost::coroutines;

class worker_coroutine : public coroutine_base
{
public:
  typedef coro::coroutine<
      void*
  >                                       coro_t;

private:
  template <typename Fn> friend struct setup;

  enum state_t 
  {
  	READY = 0,
  	RUNNING,
  	WAITING,
  	TERMINATED
  };

  struct css_data 
  {
  	void*                       vp;
  	css_cleanup_function::ptr_t cleanup_function;

  	css_data () : vp (0), cleanup_function (0) {}
  	css_data (void* vp, css_cleanup_function::ptr_t const& fn)
  	  : vp (vp)
  	  , cleanup_function (fn)
  	{
  		BOOST_ASSERT (cleanup_function);
    }

    void do_cleanup ()
    {
    	(* cleanup_function) (vp);
    }
  };

  typedef std::map<uintptr_t, css_data> css_data_t;
  typedef compat::exception_ptr exception_ptr;

  static void* null_ptr;

  css_data_t                     css_data_;
  worker_coroutine*              next_;
  clock_type::time_point         timep_;
  coro_t::pull_type*             callee_;
  coro_t::push_type              caller_;
  compat::atomic<state_t>        state_;
  compat::atomic<int>            flags_;
  compat::atomic<int>            priority_;
  exception_ptr                  except_;
  spinlock                       spinlock_;
  std::vector<worker_coroutine*> waiting_;

public:
  worker_coroutine (coro_t::pull_type* callee)
    : coroutine_base ()
    , css_data_ ()
    , next_ (0)
    , timep_ ( (clock_type::time_point::max) ())
    , callee_ (callee)
    , caller_ ()
    , state_ (READY)
    , flags_ (0)
    , priority_ (0)
    , except_ ()
    , waiting_ ()
  {
  	BOOST_ASSERT (callee_);
  }

  ~worker_coroutine ()
  {
  	BOOST_ASSERT (is_terminated ());
  	BOOST_ASSERT (waiting_.empty ());
  }

  id get_id () const BOOST_NOEXCEPT
  {
  	return id (const_cast<worker_coroutine*> (this));
  }

  int priority () const BOOST_NOEXCEPT
  {
  	return priority_;
  }

  void priority (int prio) BOOST_NOEXCEPT
  {
  	priority_ = prio;
  }

  bool detached () const BOOST_NOEXCEPT
  {
  	return 0 != (flags_.load () & flag_detached);
  }

  void detach () BOOST_NOEXCEPT
  {
  	flags_ |= flag_detached; 
  }

  bool interruption_blocked () const BOOST_NOEXCEPT
  {
  	return 0 != (flags_.load () & flag_interruption_blocked);
  }

  void interruption_blocked (bool blocked) BOOST_NOEXCEPT
  {
    if (blocked)
    	flags_ |= flag_interruption_blocked;
    else
    	flags_ &= ~flag_interruption_blocked;
  }

  bool interruption_requested () const BOOST_NOEXCEPT
  {
  	return 0 != (flags_.load () & flag_interruption_requested);
  }

  void interruption_requested (bool requested) BOOST_NOEXCEPT
  {
  	if (requested)
  		flags_ |= flag_interruption_requested;
  	else
  		flags_ &= ~flag_interruption_requested;
  }

  bool thread_affinity () const BOOST_NOEXCEPT
  {
  	return 0 != (flags_.load () & flag_thread_affinity);
  }

  void thread_affinity (bool affinity) BOOST_NOEXCEPT
  {
  	if (affinity)
  		flags_ |= flag_thread_affinity;
  	else
  		flags_ &= ~flag_thread_affinity;
  }

  bool is_terminated () const BOOST_NOEXCEPT
  {
  	return TERMINATED == state_; 
  }

  bool is_ready () const BOOST_NOEXCEPT
  {
  	return READY == state_;
  }

  bool is_running () const BOOST_NOEXCEPT
  {
  	return RUNNING == state_;
  }

  void set_terminated () BOOST_NOEXCEPT
  {
  	state_t previous = state_.exchange (TERMINATED);
  	BOOST_ASSERT (RUNNING == previous);
  }

  void set_ready () BOOST_NOEXCEPT
  {
  	state_t previous = state_.exchange (READY);
  	BOOST_ASSERT (WAITING == previous || RUNNING == previous || READY == previous);
  }

  void set_running () BOOST_NOEXCEPT
  {
  	state_t previous = state_.exchange (RUNNING);
  	BOOST_ASSERT (READY == previous);
  }

  void set_waiting () BOOST_NOEXCEPT
  {
  	state_t previous = state_.exchange (WAITING);
  	BOOST_ASSERT (RUNNING == previous);
  }

  void*
  get_css_data (void const* vp) const
  {
  	uintptr_t key (reinterpret_cast<uintptr_t> (vp));
  	css_data_t::const_iterator i (css_data_.find (key));
  	return css_data_.end () != i ? i->second.vp : 0;
  }

  void set_css_data (
    void const* vp,
    css_cleanup_function::ptr_t const& cleanup_fn,
    void* data,
    bool cleanup_existing)
  {
  	BOOST_ASSERT (cleanup_fn);

  	uintptr_t key (reinterpret_cast<uintptr_t> (vp));
  	css_data_t::iterator i (css_data_.find (key));

  	if (css_data_.end () != i)
    {
    	if (cleanup_existing) i->second.do_cleanup ();
    	if (data)
    		css_data_.insert (i, std::make_pair (key, css_data (data, cleanup_fn)));
    	else
    		css_data_.erase (i);
    }
    else
    	css_data_.insert (std::make_pair (key, css_data (data, cleanup_fn)));
  }

  exception_ptr get_exception () const BOOST_NOEXCEPT
  {
  	return except_;
  }

  void set_exception (exception_ptr except) BOOST_NOEXCEPT
  {
  	except_ = compat::move (except);
  }

  void resume (worker_coroutine* c)
  {
  	if (0 == c)
    {
    	BOOST_ASSERT (caller_);
    	BOOST_ASSERT (is_running ()); // set by the scheduler-algorithm

    	// called from main-coroutine
    	caller_ (null_ptr);
    }
    else
    {
    	// caller from worker-coroutine c
    	BOOST_ASSERT (caller_);
    	BOOST_ASSERT (is_running ()); // set by the scheduler-algorithm
    	BOOST_ASSERT (c->callee_);

    	(* c->callee_) (); // (caller_, null_ptr);
    }
  }

  void suspend ()
  {
  	BOOST_ASSERT (callee_);
  	BOOST_ASSERT (*callee_);

  	(* callee_) ();

  	BOOST_ASSERT (is_running ()); // set by the scheduler-algorithm
  }

  bool join (worker_coroutine* p)
  {
  	compat::unique_lock<spinlock> lk (spinlock_);
  	if (is_terminated ()) return false;
  	waiting_.push_back (p);
  	return true;
  }

  worker_coroutine* next () const BOOST_NOEXCEPT
  { 
  	return next_;
  }

  void next (worker_coroutine* nxt) BOOST_NOEXCEPT
  {
  	next_ = nxt;
  }

  void next_reset () BOOST_NOEXCEPT
  {
  	next_ = 0;
  }

  clock_type::time_point const& time_point () const BOOST_NOEXCEPT
  {
    return timep_;
  }

  void time_point (clock_type::time_point const& tp)
  {
    timep_ = tp;
  }

  void time_point_reset ()
  {
  	timep_ = (clock_type::time_point::max) ();
  }

  void deallocate ()
  {
  	callee_ = 0;
  	coro_t::push_type tmp (compat::move (caller_));
  }

  void release ()
  {
  	BOOST_ASSERT (is_terminated ());

  	std::vector<worker_coroutine*> waiting;

  	// get all waiting coroutines
  	spinlock_.lock ();
  	waiting.swap (waiting_);
  	spinlock_.unlock ();

  	// notify all waiting coroutines
  	BOOST_FOREACH (worker_coroutine* p, waiting)
    {
    	p->set_ready ();
    }

    // release all coroutine-specific-pointers
    BOOST_FOREACH (css_data_t::value_type& data, css_data_)
    {
    	data.second.do_cleanup ();
    }
  }
};

} // namespace detail
YAMAIL_FQNS_CONCURRENCY_COROUTINE_END
#endif // _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_WORKER_COROUTINE_H_

#ifndef _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_SCHEDULER_H_
#define _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_SCHEDULER_H_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/namespace.h>

#include <yamail/concurrency/coroutine/coroutine_manager.h>

#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>
#include <boost/utility/explicit_operator_bool.hpp>

YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN
namespace detail {

template <typename T>
class thread_local_ptr : boost::noncopyable
{
	typedef void (*cleanup_function) (T*);

	static __thread T* t_;
	cleanup_function   cf_;

public:
  thread_local_ptr (cleanup_function cf) BOOST_NOEXCEPT
    : cf_ (cf)
  {
  }

  BOOST_EXPLICIT_OPERATOR_BOOL ();

  T*   get       (    ) const BOOST_NOEXCEPT { return t_;       }
  bool operator! (    ) const BOOST_NOEXCEPT { return ! get (); }
  void reset     (T* t)       BOOST_NOEXCEPT { t_ = t;          }

  friend bool operator== (thread_local_ptr const& a, 
      thread_local_ptr const& b) BOOST_NOEXCEPT
  {
  	return a.get () == b.get ();
  }

  friend bool operator!= (thread_local_ptr const& a, 
      thread_local_ptr const& b) BOOST_NOEXCEPT
  {
  	return ! (a == b);
  }
};

template <typename T>
__thread T* thread_local_ptr<T>::t_ = 0;

class scheduler : boost::noncopyable
{
  inline static void deleter_fn (coroutine_manager* mgr) { delete mgr; }

public:
  template <typename F>
  static worker_coroutine* extract (F const& f) BOOST_NOEXCEPT
  {
  	return f.impl_;
  }

  static coroutine_manager* instance ()
  {
    static thread_local_ptr<coroutine_manager> instance_ (deleter_fn);

  	if (! instance_.get ())
  		instance_.reset (new coroutine_manager ());
    return instance_.get ();
  }

  static void replace (sched_algorithm* other)
  {
  	BOOST_ASSERT (other);
  	instance ()->set_sched_algo (other);
  }
};

} // namespace detail
YAMAIL_FQNS_CONCURRENCY_COROUTINE_END
#endif // _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_SCHEDULER_H_

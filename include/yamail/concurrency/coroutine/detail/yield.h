#ifndef _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_YIELD_H_
#define _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_YIELD_H_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/namespace.h>

#include <yamail/concurrency/spinlock.h>

#include <yamail/concurrency/coroutine/yield.h>
#include <yamail/concurrency/coroutine/error_code.h>
#include <yamail/concurrency/coroutine/detail/scheduler.h>

#include <yamail/compat/thread.h>
#include <yamail/compat/mutex.h>
#include <yamail/compat/exception.h>

#include <boost/asio/async_result.hpp>
#include <boost/asio/handler_type.hpp>

YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN
namespace detail {

template <typename T>
class yield_handler
{
public:
  yield_handler (yield_t const& y)
    : coroutine_ (scheduler::instance ()->active ())
    , ec_ (y.ec_)
    , value_ (0)
  {
  }

  void operator() (T t)
  {
  	*ec_ = error_code ();
  	*value_ = t;
  	coroutine_->set_ready ();
  }

  void operator() (error_code const& ec, T t)
  {
  	*ec_ = ec;
  	*value_ = t;
  	coroutine_->set_ready ();
  }

// private:
  coroutine_base* coroutine_;
  error_code*     ec_;
  T*              value_;
};

// Completion handler to adapt a void promise as a completion handler.
template <>
class yield_handler<void>
{
public:
  yield_handler (yield_t const& y)
    : coroutine_ (scheduler::instance ()->active ())
    , ec_ (y.ec_)
  {
  }

  void operator() ()
  {
  	*ec_ = error_code ();
  	coroutine_->set_ready ();
  }

  void operator() (error_code const& ec)
  {
  	*ec_ = ec;
  	coroutine_->set_ready ();
  }

// private: // XXX
  coroutine_base* coroutine_;
  error_code*     ec_;
};

} // namespace detail
YAMAIL_FQNS_CONCURRENCY_COROUTINE_END

namespace boost { namespace asio {

template <typename T>
class async_result<
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::yield_handler<T> >
{
public:
  typedef T type;

  explicit async_result (
    YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::yield_handler<T>& h)
  {
  	out_ec_ = h.ec_;
  	if (! out_ec_) h.ec_ = &ec_;
  	h.value_ = &value_;
  }

  type get ()
  {
  	typedef YAMAIL_FQNS_CONCURRENCY::spinlock spinlock;
  	spinlock slock_;
  	YAMAIL_FQNS_COMPAT::unique_lock<spinlock> lk (slock_);
  	YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::
  	  scheduler::instance ()->wait (lk);

  	if (! out_ec_ && ec_)
  		YAMAIL_FQNS_COMPAT::throw_exception (
  		  YAMAIL_FQNS_CONCURRENCY_COROUTINE::system_error (ec_));
  	return value_;
  }

private:
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::error_code* out_ec_;
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::error_code  ec_;
  type value_;
};

template <>
class async_result<
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::yield_handler<void> >
{
public:
  typedef void type;

  explicit async_result (
    YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::yield_handler<void>& h)
  {
  	out_ec_ = h.ec_;
  	if (! out_ec_) h.ec_ = &ec_;
  }

  void get ()
  {
  	typedef YAMAIL_FQNS_CONCURRENCY::spinlock spinlock;
  	spinlock slock_;
  	YAMAIL_FQNS_COMPAT::unique_lock<spinlock> lk (slock_);
  	YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::
  	  scheduler::instance ()->wait (lk);

  	if (! out_ec_ && ec_)
  		YAMAIL_FQNS_COMPAT::throw_exception (
  		  YAMAIL_FQNS_CONCURRENCY_COROUTINE::system_error (ec_));
  }

private:
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::error_code* out_ec_;
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::error_code  ec_;
};

// Handler type specialisation for use_future.
// TODO

}} // namespace boost::asio
#endif // _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_YIELD_H_

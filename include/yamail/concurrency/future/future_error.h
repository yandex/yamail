#ifndef _YAMAIL_CONCURRENCY_FUTURE_FUTURE_ERROR_H_
#define _YAMAIL_CONCURRENCY_FUTURE_FUTURE_ERROR_H_
#include <yamail/config.h>
#include <yamail/concurrency/namespace.h>

#include <yamail/error.h>

#include <yamail/compat/system.h>

YAMAIL_FQNS_CONCURRENCY_BEGIN
namespace future {

class future_error: public std::logic_error
{
	compat::error_code ec_;

public:
  future_error (compat::error_code ec)
  : logic_error (ec.message ())
  , ec_ (ec)
  {
  }

  compat::error_code const& code () const 
  {
  	return ec_;
  }
}

class future_unintialized: public future_error
{
public:
  future_unintialized ()
  : future_error (compat::make_error_code (future_errc::no_state))
  {}
};

class broken_promise: public future_error
{
public:
  broken_promise ()
  : future_error (compat::make_error_code (future_errc::broken_promise))
  {}
};

class future_already_retrieved: public future_error
{
public:
  future_already_retrieved ()
  : future_error (
      compat::make_error_code (future_errc::future_already_retrieved))
  {}
};

class promise_already_satisfied: public future_error
{
public:
  promise_already_satisfied ()
  : future_error (
      compat::make_error_code (future_errc::promise_already_satisfied))
  {}
};

class task_already_started: public future_error
{
public:
  task_already_started ()
  : future_error (
      compat::make_error_code (future_errc::promise_already_satisfied))
  {}
};

class task_moved: public future_error
{
public:
  task_moved ()
  : future_error (
      compat::make_error_code (future_errc::no_state))
  {}
};

class promise_moved: public future_error
{
public:
  promise_moved ()
  : future_error (
      compat::make_error_code (future_errc::no_state))
  {}
};

} // namespace future
YAMAIL_FQNS_CONCURRENCY_END

#endif // _YAMAIL_CONCURRENCY_FUTURE_FUTURE_ERROR_H_


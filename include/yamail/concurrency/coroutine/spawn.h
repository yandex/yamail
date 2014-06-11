#ifndef _YAMAIL_CONCURRENCY_COROUTINE_SPAWN_H_
#define _YAMAIL_CONCURRENCY_COROUTINE_SPAWN_H_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/namespace.h>

#include <yamail/concurrency/coroutine/error_code.h>
#include <yamail/concurrency/coroutine/attributes.h>

#include <boost/asio/detail/config.hpp>
#include <boost/asio/detail/weak_ptr.hpp>
#include <boost/asio/detail/wrapped_handler.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN

template <typename Handler>
class basic_yield_context
{
public:
  basic_yield_context (
      detail::coroutine_base* crb,
      Handler& handler)
    : coroutine_ (crb)
    , handler_ (handler)
    , ec_ (0)
  {
  }

  basic_yield_context operator[] (error_code& ec)
  {
  	basic_yield_context tmp (*this);
  	tmp.ec_ = &ec;
  	return tmp;
  }

private:
  detail::coroutine_base* coroutine_;
  Handler&                handler_;
  error_code*             ec_;
};

typedef basic_yield_context<
  boost::asio::detail::wrapped_handler<
      boost::asio::io_service::strand
    , void(*)()
    , boost::asio::detail::is_continuation_if_running
  >
> yield_context;


template <typename Handler, typename Function>
void spawn (boost::asio::io_service& io_service,
        BOOST_ASIO_MOVE_ARG (Handler) handler,
        BOOST_ASIO_MOVE_ARG (Function) function,
        attributes const& attr = attributes ());

template <typename Handler, typename Function>
void spawn (boost::asio::io_service& io_service,
        basic_yield_context<Handler> ctx,
        BOOST_ASIO_MOVE_ARG (Function) function,
        attributes const& attr = attributes ());

template <typename Handler, typename Function>
void spawn (boost::asio::io_service::strand strand,
        BOOST_ASIO_MOVE_ARG (Function) function,
        attributes const& attr = attributes ());

template <typename Handler, typename Function>
void spawn (boost::asio::io_service& io_service,
        BOOST_ASIO_MOVE_ARG (Function) function,
        attributes const& attr = attributes ());

YAMAIL_FQNS_CONCURRENCY_COROUTINE_END

#include <yamail/concurrency/coroutine/detail/spawn.h>

#endif // _YAMAIL_CONCURRENCY_COROUTINE_SPAWN_H_

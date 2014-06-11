#ifndef _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_SPAWN_H_
#define _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_SPAWN_H_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/namespace.h>

#include <boost/noncopyable.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/detail/config.hpp>
#include <boost/asio/detail/handler_alloc_helpers.hpp>
#include <boost/asio/detail/handler_cont_helpers.hpp>
#include <boost/asio/detail/handler_invoke_helpers.hpp>
#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/shared_ptr.hpp>
#include <boost/asio/handler_type.hpp>

#include <yamail/concurrency/spinlock.h>
#include <yamail/concurrency/coroutine/error_code.h>
#include <yamail/concurrency/coroutine/detail/scheduler.h>

#include <yamail/compat/mutex.h>
#include <yamail/compat/shared_ptr.h>

YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN
namespace detail {

template <typename Handler, typename T>
class coroutine_handler
{
public:
  coroutine_handler (basic_yield_context<Handler> ctx)
    : coroutine_ (ctx.coroutine_)
    , handler_ (ctx.handler_)
    , ec_ (ctx.ec_)
    , value_ (0)
  {
  }

  void operator() (T value)
  {
  	*ec_ = error_code ();
  	*value_ = value;
  	coroutine_->set_ready ();
  }

  void operator() (error_code ec, T value)
  {
  	*ec_ = ec;
  	*value_ = value;
  	coroutine_->set_ready ();
  }

// private:
  detail::coroutine_base* coroutine_;
  Handler&                handler_;
  error_code*             ec_;
  T                       value_;
};

template <typename Handler>
class coroutine_handler<Handler, void>
{
public:
  coroutine_handler (basic_yield_context<Handler> ctx)
    : coroutine_ (ctx.coroutine_)
    , handler_ (ctx.handler_)
    , ec_ (ctx.ec_)
  {
  }

  void operator() ()
  {
  	*ec_ = error_code ();
  	coroutine_->set_ready ();
  }

  void operator() (error_code ec)
  {
  	*ec_ = ec;
  	coroutine_->set_ready ();
  }

// private:
  detail::coroutine_base* coroutine_;
  Handler&                handler_;
  error_code*             ec_;
};

template <typename Handler, typename T>
void* 
asio_handler_allocate (std::size_t size, 
        coroutine_handler<Handler,T>* this_handler)
{
	return boost_asio_handler_alloc_helpers::allocate (
	        size, this_handler->handler_);
}

template <typename Handler, typename T>
void asio_handler_deallocate (void* pointer, std::size_t size,
        coroutine_handler<Handler,T>* this_handler)
{
	boost_asio_handler_alloc_helpers::deallocate (
	        pointer, size, this_handler->handler_);
}

template <typename Handler, typename T>
bool asio_handler_is_continuation (coroutine_handler<Handler,T>*)
{
	return true;
}

template <typename Function, typename Handler, typename T>
void asio_handler_invoke (Function& function, 
        coroutine_handler<Handler,T>* this_handler)
{
	boost_asio_handler_invoke_helpers::invoke (
	        function, this_handler->handler_);
}

template <typename Function, typename Handler, typename T>
void asio_handler_invoke (Function const& function, 
        coroutine_handler<Handler,T>* this_handler)
{
	boost_asio_handler_invoke_helpers::invoke (
	        function, this_handler->handler_);
}

} // namespace detail
YAMAIL_FQNS_CONCURRENCY_COROUTINE_END

namespace boost { namespace asio {

template <typename Handler, typename ReturnType>
struct handler_type<
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::basic_yield_context<Handler>,
  ReturnType ()
>
{
	typedef
    YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::coroutine_handler<Handler, void>
	    type;
};

template <typename Handler, typename ReturnType, typename Arg1>
struct handler_type<
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::basic_yield_context<Handler>,
  ReturnType (Arg1)
>
{
	typedef
    YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::coroutine_handler<Handler,Arg1>
	    type;
};

template <typename Handler, typename ReturnType>
struct handler_type<
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::basic_yield_context<Handler>,
  ReturnType (YAMAIL_FQNS_CONCURRENCY_COROUTINE::error_code)
>
{
	typedef
    YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::coroutine_handler<Handler, void>
	    type;
};

template <typename Handler, typename ReturnType, typename Arg2>
struct handler_type<
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::basic_yield_context<Handler>,
  ReturnType (YAMAIL_FQNS_CONCURRENCY_COROUTINE::error_code, Arg2)
>
{
	typedef
    YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::coroutine_handler<Handler,Arg2>
	    type;
};

template <typename Handler, typename T>
class async_result<
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::coroutine_handler<Handler,T>
>
{
public:
  typedef T type;

  explicit async_result (
    YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::coroutine_handler<Handler,T>& h)
    : out_ec_ (0)
    , ec_ ()
    , value_ ()
  {
  	out_ec_ = h.ec_;
  	if (! out_ec_) h.ec_ = &ec_;
  	h.value_ = &value_;
  }

  type get ()
  {
  	using YAMAIL_FQNS_CONCURRENCY::spinlock;
  	spinlock slock;
  	YAMAIL_FQNS_COMPAT::unique_lock<spinlock> lk (slock);
  	YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::
  	  scheduler::instance ()->wait (lk);

  	if (! out_ec_ && ec_)
  		throw YAMAIL_FQNS_CONCURRENCY_COROUTINE::system_error (ec_);
  	return value_;
  }

private:
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::error_code* out_ec_;
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::error_code  ec_;
  type                                           value_;
};

template <typename Handler>
class async_result<
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::coroutine_handler<Handler,void>
>
{
public:
  typedef void type;

  explicit async_result (YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::
      coroutine_handler<Handler,void>& h)
    : out_ec_ (0)
    , ec_ ()
  {
  	out_ec_ = h.ec_;
  	if (! out_ec_) h.ec_ = &ec_;
  }

  type get ()
  {
  	using YAMAIL_FQNS_CONCURRENCY::spinlock;
  	spinlock slock;
  	YAMAIL_FQNS_COMPAT::unique_lock<spinlock> lk (slock);
  	YAMAIL_FQNS_CONCURRENCY_COROUTINE::detail::
  	  scheduler::instance ()->wait (lk);

  	if (! out_ec_ && ec_)
  		throw YAMAIL_FQNS_CONCURRENCY_COROUTINE::system_error (ec_);
  }

private:
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::error_code* out_ec_;
  YAMAIL_FQNS_CONCURRENCY_COROUTINE::error_code  ec_;
};

}} // namespace boost::asio


YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN
namespace detail {

template <typename Handler, typename Function>
struct spawn_data: boost::noncopyable
{
	spawn_data (boost::asio::io_service& io_svc, 
      BOOST_ASIO_MOVE_ARG(Handler) handler,
      bool call_handler, 
      BOOST_ASIO_MOVE_ARG(Function) function)
	  : handler_ (BOOST_ASIO_MOVE_CAST (Handler) (handler))
	  , call_handler_ (call_handler)
	  , function_ (BOOST_ASIO_MOVE_CAST (Function) (function))
	{
  }

  boost::asio::io_service io_svc_;
  coroutine_base*         coroutine_;
  Handler                 handler_;
  bool                    call_handler_;
  Function                function_;
};

template <typename Handler, typename Function>
struct coroutine_entry_point
{
	void operator() ()
	{
		compat::shared_ptr<spawn_data<Handler,Function> > data (data_);
		data->coroutine_ = scheduler::instance ()->active ();
    basic_yield_context<Handler> const yield ( data->fiber_, data->handler_);

    boost::asio::io_service::work w (data->io_svc_);

    (data->function_) (yield);

    if (data->call_handler_)
    	(data->handler_) ();
  }

  compat::shared_ptr<spawn_data<Handler, Function> > data_;
};

template <typename Handler, typename Function>
struct spawn_helper
{
	void operator() ()
	{
		coroutine_entry_point<Handler,Function> entry_point = { data_ };
		coroutine::coroutine crt (entry_point, attributes);
		crt.detach ();
  }

  compat::shared_ptr<spawn_data<Handler, Function> > data_;
  attributes                                         attributes_;
};

inline void default_spawn_handler () {}

} // namespace detail

template <typename Handler, typename Function>
void spawn (boost::asio::io_service& io_service,
        BOOST_ASIO_MOVE_ARG (Handler) handler,
        BOOST_ASIO_MOVE_ARG (Function) function,
        attributes const& attr = attributes ())
{
	detail::spawn_helper<Handler,Function> helper;

	helper.data_.reset (
	  new detail::spawn_data<Handler, Function> (
	      io_service, 
	      BOOST_ASIO_MOVE_CAST (Handler) (handler), true
	      BOOST_ASIO_MOVE_CAST (Function) (function)
	  )
	);

	helper.attributes_ = attr;

	boost_asio_handler_invoke_helpers::invoke (
	    helper, helper.data_->handler_);
}

template <typename Handler, typename Function>
void spawn (boost::asio::io_service& io_service,
        basic_yield_context<Handler> ctx,
        BOOST_ASIO_MOVE_ARG (Function) function,
        attributes const& attr = attributes ())
{
	Handler handler( ctx.handler_); // Explicit copy that might be moved from.
	detail::spawn_helper<Handler,Function> helper;

	helper.data_.reset (
	  new detail::spawn_data<Handler, Function> (
	      io_service, 
	      BOOST_ASIO_MOVE_CAST (Handler) (handler), false
	      BOOST_ASIO_MOVE_CAST (Function) (function)
	  )
	);

	helper.attributes_ = attr;

	boost_asio_handler_invoke_helpers::invoke (
	    helper, helper.data_->handler_);
}


template <typename Handler, typename Function>
void spawn (boost::asio::io_service::strand strand,
        BOOST_ASIO_MOVE_ARG (Function) function,
        attributes const& attr = attributes ())
{
	spawn (
	  strand.get_io_service (),
	  strand.wrap (& detail::default_spawn_handler),
	  BOOST_ASIO_MOVE_CAST (Function) (function),
	  attr
	);
}

template <typename Handler, typename Function>
void spawn (boost::asio::io_service& io_service,
        BOOST_ASIO_MOVE_ARG (Function) function,
        attributes const& attr = attributes ())
{
	spawn (
	  boost::asio::io_service::strand (io_service),
	  BOOST_ASIO_MOVE_CAST (Function) (function),
	  attr
	);
}

YAMAIL_FQNS_CONCURRENCY_COROUTINE_END
#endif // _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_SPAWN_H_

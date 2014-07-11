#ifndef _YAMAIL_CONCURRENCY_FUTURE_FUTURE_HPP_
#define _YAMAIL_CONCURRENCY_FUTURE_FUTURE_HPP_ 1
#include <yamail/config.h>
#include <yamail/concurrency/namespace.h>

#include <yamail/error.h>

#include <yamail/compat/function.h>
#include <yamail/compat/shared_ptr.h>
#include <yamail/compat/bind.h>
#include <yamail/compat/move.h>
#include <yamail/compat/type_traits.h> // result_of

#include <yamail/concurrency/future/future_exceptions.h>
#include <yamail/concurrency/future/future_detail.h>

//  Copyright (c) 2007 Braddock Gaskill Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//  exception_ptr.hpp/cpp copyright Peter Dimov 

// #include <boost/utility/result_of.hpp>
// #include <boost/exception/all.hpp>

// TODO: use compat type traits
#include <boost/type_traits/is_base_of.hpp>

#ifdef __GNUC__
#define DEPRECATED __attribute__ ((deprecated))
#else
#define DEPRECATED
#endif

YAMAIL_FQNS_CONCURRENCY_BEGIN
namespace future {

class callback_reference;

class untyped_promise {
  public:
    untyped_promise() {}
    explicit untyped_promise(const YAMAIL_FQNS_COMPAT::shared_ptr<detail::future_impl> &fimpl) : f_(fimpl) {}
    untyped_promise(const untyped_promise &other) : f_(other.f_) {}
    untyped_promise& operator=(const untyped_promise& t) {f_ = t.f_; return *this;}

    template<typename E>
    void set_exception_core( E const & e,
            const ::boost::true_type& /*is_base_of<error, E>*/ ) { // stores the exception e and transitions to ready()
      f_->set_exception(e.exception_ptr());
    }
    template<typename E>
    void set_exception_core( E const & e,
            const ::boost::false_type& /*is_base_of<error, E>*/ ) { // stores the exception e and transitions to ready()
      f_->set_exception(YAMAIL_FQNS_COMPAT::make_exception_ptr (e));
    }

#if __cplusplus >= 201103L
    template<typename E> void set_exception( E&& e ) {
      this->set_exception_core(std::forward<E> (e), 
          ::boost::is_base_of<YAMAIL_FQNS::error, E>());
    }
#else
    template<typename E> void set_exception( E const & e ) {
      this->set_exception_core(e, ::boost::is_base_of<YAMAIL_FQNS::error, E>());
    }
#endif
    // Attempt's a 'throw', assuming there is an exception set
    void set_current_exception() {
      f_->set_exception(YAMAIL_FQNS_COMPAT::current_exception());
    }

    void set_exception( const YAMAIL_FQNS_COMPAT::exception_ptr & e) {
      f_->set_exception(e);
    }

#if __cplusplus >= 201103L
    void set_exception(YAMAIL_FQNS_COMPAT::exception_ptr&& e) {
      f_->set_exception(std::move (e));
    }
#endif

    // Add a cancel handler, which will be invoked if
    // future&lt;T&gt;::cancel() is ever called
    // Note: If the future is ALREADY canceled, then get_cancel_handler()
    // will call the handler immediately.
    // There is only one cancel_handler() for an underlying Future instance
    void DEPRECATED set_cancel_handler( 
            const YAMAIL_FQNS_COMPAT::function<void (void)> &f ) {
      f_->set_cancel_handler(f);
    }

    void DEPRECATED unset_cancel_handler() {
      f_->set_cancel_handler(YAMAIL_FQNS_COMPAT::function<void (void)>());
    }

  protected:
    YAMAIL_FQNS_COMPAT::shared_ptr<detail::future_impl> f_;
};

template<typename R> class promise : public untyped_promise
{
  public:
    promise() : impl_(new detail::promise_impl<R>) 
    {f_ = impl_->f_;}; // creates an empty future
    
    promise(const promise& t) : untyped_promise(t.impl_->f_), impl_(t.impl_) {}
    
    promise& operator=(const promise& t) {
      impl_ = t.impl_; 
      untyped_promise::operator=(t);
      return *this;
    }

    void set( R const& r ) { // sets the value r and transitions to ready()
      impl_->f_->set_value(r, *impl_->value_);
    }
#if __cplusplus >= 201103L
    void set (R&& r) {
    	impl_->f_->set_value (std::forward<R> (r), *impl_->value_);
    }
#endif

    void set_or_throw( R const & r) {
      if (!impl_->f_->set_value(r, *impl_->value_))
        throw future_already_set();
    }

    bool is_needed() {return impl_->f_->is_needed();}
    void wait_until_needed() {return impl_->f_->wait_until_needed();}
    YAMAIL_FQNS_COMPAT::shared_ptr<detail::future_impl> get_needed_future() 
    {return impl_->f_->get_needed_future();}

    void reset() {
      impl_.reset();
      f_.reset();
    }
  private:
    template <typename Y> friend class future;
    YAMAIL_FQNS_COMPAT::shared_ptr<detail::promise_impl<R> > impl_;
};

template <typename R> class future_unwrapper {};

template <typename> class future;
template<typename R2> class future_unwrapper<future<R2> >
{
	typedef future<future<R2> > inner_future_type;

	class setter
	{
		inner_future_type& inner_;
		promise<R2> proxy_;

	public:
		setter (inner_future_type& inner, promise<R2> const& proxy) 
		  : inner_ (inner)
		  , proxy_ (proxy)
		{
    }

    void operator() ()
    {
    	try {
    	  proxy_.set (inner_.get ());
      }
      catch (...)
      {
      	proxy_.set_exception (YAMAIL_FQNS_COMPAT::current_exception ());
      }
    }
  };

public:
	inner_future_type      & this_future ()       
	{ return *static_cast<inner_future_type*> (this); }

	inner_future_type const& this_future () const 
	{ return *static_cast<inner_future_type const*> (this); }

	future<R2> unwrap ()
	{
    promise<R2> prom;

    this_future ().add_callback (setter (this_future (), prom));
    return prom;
  }
};

template<typename R> class future : public future_unwrapper<R>
{
  public:
    //future() : impl_(new detail::future_impl<R>) {}; // creates an empty future

    // Default constructor will create a future, and will immediately set a
    // broken_promise exception.
    // A default-constructed future is only good for equality assignment to a
    // valid future.
    future() : impl_(), value_() {
      promise<R> p;
      impl_ = p.impl_->f_;
      value_ = p.impl_->value_;
    }

    future(const future& t) : impl_(t.impl_), value_(t.value_) {}

    template<typename T>
      future(const future<T>& t) : 
        impl_(t.impl_), 
        value_(new detail::return_value_type_adaptor<R,T>(t.value_)) 
  {}

    future(const promise<R>& p) : impl_(p.impl_->f_), value_(p.impl_->value_) {}

    template<typename T>
      future(const promise<T>& p) 
      : impl_(p.impl_->f_), 
      value_(new detail::return_value_type_adaptor<R,T>(p.impl_->value_)) 
  {}

  protected:
    // used by future<void> for typeless futures
    future(const YAMAIL_FQNS_COMPAT::shared_ptr<detail::future_impl> &impl) :
      impl_(impl),
      value_(new detail::return_value_real<R>(-999)) //value is never used
  {}
  public:

    future& operator=(const future& t) {
      impl_ = t.impl_; 
      value_ = t.value_;
      return *this;
    }

    template<typename T>
      future<R>& operator=(const future<T>& t) {
        impl_ = t.impl_; 
        value_ = YAMAIL_FQNS_COMPAT::shared_ptr<detail::return_value_base<R> >(
            new detail::return_value_type_adaptor<R,T>(t.value_));
        return *this;
      }
    ~future() {};

    bool has_value() const { // newer Dimov proposal N2185
      return impl_->has_value();
    }

    bool has_exception() const { // N2185
      return impl_->has_exception();
    }

    bool ready() const { // queries whether the future contains a value or an exception
      return impl_->ready();
    }

    void wait() { // wait for ready()
      return impl_->wait();
    }

    template <typename Clock, typename Duration>
    bool timed_wait(
      YAMAIL_FQNS_COMPAT::chrono::time_point<Clock,Duration> const & abstime ) 
    {
      return impl_->timed_wait(abstime);
    }

    template <typename Rep, typename Period>
    bool timed_wait(
      YAMAIL_FQNS_COMPAT::chrono::duration<Rep,Period> const & reltime ) 
    {
      return impl_->timed_wait(reltime);
    }

    operator R() const { // N2185
      return impl_->get(*value_);
    }

    R get() const {
      return impl_->get(*value_);
    }

    R operator()() const { // waits for a value, then returns it
      return impl_->get(*value_);
    }

    void set_needed() const {
      impl_->set_needed();
    }

    // set future exception to yplatform::future::future_cancel, and call
    // the cancel handler if one has been set by the user
    void DEPRECATED cancel() {
      impl_->cancel();
    }

    template <typename F, typename P>
    struct assign_future 
    {
    	future this_future;
    	F func;
    	P prom;
    	assign_future (future const& fu, F f, P p) 
    	  : this_future (fu)
    	  , func (YAMAIL_FQNS_COMPAT::move (f))
    	  , prom (YAMAIL_FQNS_COMPAT::move (p)) 
    	{
    	}

    	void operator() ()
    	{
    		try {
    		  set_prom<F> ();
    		}
    		catch (...)
    		{
          prom.set_exception (YAMAIL_FQNS_COMPAT::current_exception());
        }
      }

      template <typename FF,
        typename boost::enable_if<
          boost::is_same< 
              typename YAMAIL_FQNS_COMPAT::result_of<FF(future&)>::type
            , void
          >, int>::type = 0>
      void set_prom ()
      {
    	  func (this_future);
    	  prom.set ();
      }

      template <typename FF,
        typename boost::disable_if<
          boost::is_same< 
              typename YAMAIL_FQNS_COMPAT::result_of<FF(future&)>::type
            , void
          >, int>::type = 0>
      void set_prom ()
      {
    	  prom.set (func (this_future));
      }
    };

    template <typename F>
    future<typename YAMAIL_FQNS_COMPAT::result_of<F(future&)>::type>
#if YAMAIL_CPP >= 11
    then (F&& func) // C++11 required
#else
    then (F func) 
#endif
    {
    	typedef typename YAMAIL_FQNS_COMPAT::result_of<F(future&)>::type
    	  func_return_type;

    	typedef promise<func_return_type> promise_type;

    	promise_type prom;

      add_callback (
        assign_future<F, promise_type> (*this, 
#if YAMAIL_CPP >= 11
          std::forward<F> (func), 
#else
          YAMAIL_FQNS_COMPAT::move (func),
#endif
          prom)
      );

    	return prom;
    }

    callback_reference add_callback(
        const YAMAIL_FQNS_COMPAT::function<void (void)> &f) 
    {
      return impl_->add_callback(f);
    }

    // remove_callback will remove a registered callback
    // Calling with an invalid callback_reference, or a
    // callback_reference which has already been removed is
    // undefined.
    // This function is guaranteed not to return until the
    // callback is removed.
    // This can block if callbacks are already in progress
    void remove_callback(callback_reference &ref) {
      impl_->remove_callback(ref);
    }
  private:
    template <typename Y> friend class future;
    YAMAIL_FQNS_COMPAT::shared_ptr<detail::future_impl > impl_;
    YAMAIL_FQNS_COMPAT::shared_ptr<detail::return_value_base<R> > value_;
};

// note, promise<int> must be public for friend to work in specialization (?)
template<> class promise<void> : public promise<int> {
  private:
    typedef promise<int> base_type;
  public:
    promise() : promise<int>() {}
    promise(const promise& t) : promise<int>(t) {}
    promise& operator=(const promise& t) {
      base_type::operator=(t);
      return *this;
    }
    using base_type::set_exception;
    using base_type::set_cancel_handler;
    using base_type::is_needed;
    using base_type::wait_until_needed;
    void set() {
      base_type::set(0);
    }
    void set_or_throw() {
      base_type::set_or_throw(0);
    }
};

// void specialization, based on Peter Dimov's example
template<> class future<void> : private future<int> {
  private:
    typedef future<int> base_type;
  public:
    future() : base_type() {}
    future(const future& t) 
      : base_type(static_cast<const future<int>&> (t)) 
    {}

    future(const promise<void> &p) 
      : base_type(static_cast<const promise<int>&> (p)) 
    {}

    future(const YAMAIL_FQNS_COMPAT::shared_ptr<detail::future_impl> &impl) 
      : base_type(impl) 
    {}

    template<typename T>
      future(const future<T> &t) : base_type(t.impl_) {}

    template<typename T>
      future(const promise<T> &t) : base_type(t.impl_->f_) {} 

    future& operator=(const future& t) {
      base_type::operator=(static_cast<const future<int>&> (t));
      return *this;
    }

    template<typename T>
      future& operator=(const future<T>& t) {
        future<void> tmp(t);
        base_type::operator=(static_cast<const future<int>&> (tmp));
        return *this;
      }

    using base_type::has_value;
    using base_type::has_exception;
    using base_type::timed_wait;
    using base_type::cancel;
    using base_type::ready;
    using base_type::wait;
    using base_type::set_needed;
    using base_type::add_callback;
    using base_type::then;
//    using base_type::unwrap;
    using base_type::remove_callback;

    void get() const {
      base_type::get();
    }
};

template <typename R > class promise< R& > : public promise<R*> {
  private:
    typedef promise< R* > base_type;
  public:
    promise() : promise<R*>() {}
    promise(const promise& t) : promise<R*>(t) {}
    promise& operator=(const promise& t) {
      base_type::operator=(t);
      return *this;
    }
    using base_type::set_exception;
    using base_type::set_cancel_handler;
    using base_type::is_needed;
    using base_type::wait_until_needed;
    void set(R &r) {
      base_type::set(&r);
    }
    void set_or_throw(R &r) {
      base_type::set_or_throw(&r);
    }
};

// reference passing specialization, based on Peter Dimov's example
template<typename R > class future< R& >: private future< R* >
{
  private:
    typedef future< R* > base_type;
  public:
    future() : base_type() {}
    future(const future& t) 
      : base_type(static_cast<const future<R*> &> (t)) 
    {}

    future(const promise<R*> &p) 
      : base_type(static_cast<const promise<R*> &> (p)) 
    {}

    future& operator=(const future& t) {
      base_type::operator=(t);
      return *this;
    }
    using base_type::has_value;
    using base_type::has_exception;
    using base_type::timed_wait;
    using base_type::cancel;
    using base_type::ready;
    using base_type::wait;
    using base_type::add_callback;
    using base_type::then;
    using base_type::unwrap;
    using base_type::set_needed;

    operator R&() const {
      return *base_type::get();
    }

    R& get() const {
      return *base_type::get();
    }
};

template<typename R> class future_wrapper
{
  public:
    future_wrapper(const YAMAIL_FQNS_COMPAT::function<R (void)> &fn, const promise<R> &ft )
      : fn_(fn), ft_(ft) {}; // stores fn and ft

    void operator()() throw() 
    { // executes fn() and places the outcome into ft
      try {
        ft_.set(fn_());
      } catch (...) {
        ft_.set_exception(YAMAIL_FQNS_COMPAT::current_exception());
      }
    }
    future<R> get_future() const {return future<R>(ft_);}
  private:
    YAMAIL_FQNS_COMPAT::function<R (void)> fn_;
    promise<R> ft_;
};

// void specialization
template<> class future_wrapper<void>
{
  public:
    future_wrapper(const YAMAIL_FQNS_COMPAT::function<void (void)> &fn, const promise<void> &ft ) : ft_(ft), fn_(fn) {}; // stores fn and ft
    void operator()() throw() { // executes fn() and places the outcome into ft
      try {
        fn_();
        ft_.set();
      } catch (...) {
        ft_.set_exception(YAMAIL_FQNS_COMPAT::current_exception());
      }
    }
    future<void> get_future() const {return future<void>(ft_);}
  private:
    promise<void> ft_;
    YAMAIL_FQNS_COMPAT::function<void (void)> fn_;
};

#if YAMAIL_CPP >= 11
template <typename T>
future<typename YAMAIL_FQNS_COMPAT::type_traits::decay<T>::type>
make_future (T&& value)
{
	promise<typename YAMAIL_FQNS_COMPAT::type_traits::decay<T>::type> ret;
	ret.set (std::forward<T> (value));
	return ret;
}
#else
template <typename T>
future<typename YAMAIL_FQNS_COMPAT::type_traits::decay<T>::type>
make_future (T value)
{
	promise<typename YAMAIL_FQNS_COMPAT::type_traits::decay<T>::type> ret;
	ret.set (YAMAIL_FQNS_COMPAT::move<T> (value));
	return ret;
}
#endif

} // namespace future
YAMAIL_FQNS_CONCURRENCY_END

#endif // _YAMAIL_CONCURRENCY_FUTURE_FUTURE_HPP_


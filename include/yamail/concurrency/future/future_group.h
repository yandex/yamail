#ifndef _YAMAIL_CONCURRENCY_FUTURE_FUTURE_FUTURE_GROUP_HPP_
#define _YAMAIL_CONCURRENCY_FUTURE_FUTURE_FUTURE_GROUP_HPP_
#include <yamail/config.h>
#include <yamail/concurrency/namespace.h>
#include <yamail/concurrency/future/future.h>

#include <yamail/compat/shared_ptr.h>
#include <yamail/compat/exception.h>
#include <yamail/compat/mutex.h>

#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>


YAMAIL_FQNS_CONCURRENCY_BEGIN
namespace future {

class future_or_func_impl : ::boost::noncopyable {
  public:
    future_or_func_impl(future<void> a, future<void> b) : a_(a), b_(b) {}
    // future_or_func_impl(const future_or_func_impl &f) : a_(f.a_), b_(f.b_), p_(f.p_) {}
    struct future_or_failed : public std::exception { };
    void operator()() {
YAMAIL_FQNS_COMPAT::lock_guard<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
try {
  if (a_.ready()) {
    a_.get(); //throw if you have any
    p_.set();
  } else if (b_.ready()) {
    b_.get();
    p_.set();
  }
} catch (...) {
  p_.set_exception(YAMAIL_FQNS_COMPAT::current_exception());
}
    }
    future<void> a_,b_;
    promise<void> p_;
    ::boost::optional<callback_reference> a_cb_;
    ::boost::optional<callback_reference> b_cb_;
    YAMAIL_FQNS_COMPAT::mutex mutex_;
};

class future_or_func {
  public:
    future_or_func(future<void> a, future<void> b) : impl_(new future_or_func_impl(a, b)) {}
    // future_or_func(const future_or_func &f) : impl_(new future_or_func_impl(*f.impl_)) {}
    void operator()() {
      (*impl_)();
    }
    promise<void> get_promise() {return impl_->p_;}
  protected:
    YAMAIL_FQNS_COMPAT::shared_ptr<future_or_func_impl> impl_;
};

class future_and_func_impl : ::boost::noncopyable {
  public:
    future_and_func_impl(future<void> a, future<void> b) : a_(a), b_(b), got_a_(false), got_b_(false) {}
    // future_and_func_impl(const future_and_func_impl &f) : a_(f.a_), b_(f.b_), p_(f.p_), got_a_(false), got_b_(false) {}
    struct future_and_failed : public std::exception { };
    void operator()() {
YAMAIL_FQNS_COMPAT::lock_guard<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
try {
  if ((!got_a_) && a_.ready()) {
    a_.get(); //throw if you have any
    got_a_ = true;
  } 
  if ((!got_b_) && b_.ready()) {
    b_.get(); //throw if you have any
    got_b_ = true;
  }
} catch (...) {
  p_.set_exception(YAMAIL_FQNS_COMPAT::current_exception());
}
if (got_a_ && got_b_) {
  p_.set(); //completed
      }
    }
    future<void> a_,b_;
    promise<void> p_;
    bool got_a_, got_b_;
    YAMAIL_FQNS_COMPAT::mutex mutex_;
};

class future_and_func {
  public:
    future_and_func(future<void> a, future<void> b) : impl_(new future_and_func_impl(a, b)) {}
    // future_and_func(const future_and_func &f) : impl_(new future_and_func_impl(*f.impl_)) {}
    void operator()() {
      (*impl_)();
    }
    promise<void> get_promise() {return impl_->p_;}
  protected:
    YAMAIL_FQNS_COMPAT::shared_ptr<future_and_func_impl> impl_;
};

template<typename T>
  class comb {
    public:
comb(const comb &t) : f_(t.f_) {}
comb(const future<T> &f) : f_(f) {}
comb(const promise<T> &p) : f_(p) {}
operator future<T> () const {
  return f_;
  }	  
const future<T> &get() const {
  return f_;
}
    private:
future<T> f_;
  };

template<typename T, typename U>
  future<void> future_or(future<T> &a, future<U> &b) {
    //future_or_func fa(future<void>(a), future<void>(b));
    future<void> va(a);
    future<void> vb(b);
    future_or_func fa(va,vb);

    a.add_callback(fa);
    b.add_callback(fa);
    fa(); // check if already satisfied
    return fa.get_promise();
  }

template<typename T, typename U>
comb<void> operator||(const comb<T> &a, const comb<U> &b) {
  future<T> fa = a.get();
  future<U> fb = b.get();
  return future_or(fa, fb);
}

template<typename T, typename U>
  future<void> future_and(future<T> &a, future<U> &b) {
    //future_or_func fa(future<void>(a), future<void>(b));
    future<void> va(a);
    future<void> vb(b);
    future_and_func fa(va,vb);

    a.add_callback(fa);
    b.add_callback(fa);
    fa(); // check if already satisfied
    return fa.get_promise();
  }

template<typename T, typename U>
comb<void> operator&&(const comb<T> &a, const comb<U> &b) {
  future<T> fa = a.get();
  future<U> fb = b.get();
  return future_and(fa, fb);
}

class future_group {
  public:
    future_group(const future<void> &f) : fut_(f) {}
    operator future<void> () {
return fut_;
    } 
  private:
    future<void> fut_;
};

template<typename T>
  comb<T> op(const future<T> &x) {
    return comb<T>(x);
  }

template<typename T>
  comb<T> op(const promise<T> &x) {
    return comb<T>(x);
  }

}
YAMAIL_FQNS_CONCURRENCY_END
#endif // _YAMAIL_CONCURRENCY_FUTURE_FUTURE_FUTURE_GROUP_HPP_

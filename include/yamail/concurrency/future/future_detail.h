#ifndef _YAMAIL_CONCURRENCY_FUTURE_FUTURE_DETAIL_HPP_
#define _YAMAIL_CONCURRENCY_FUTURE_FUTURE_DETAIL_HPP_
#include <yamail/config.h>
#include <yamail/concurrency/namespace.h>
#include <yamail/compat/shared_ptr.h>
#include <yamail/compat/chrono.h>
#include <yamail/compat/function.h>
#include <yamail/compat/mutex.h>
#include <yamail/compat/exception.h>

#include <vector>
#include <list>

// DEBUG
// #include <iostream>

YAMAIL_FQNS_CONCURRENCY_BEGIN
namespace future {
namespace detail {
class future_impl;
}

class callback_reference {
  public:
    callback_reference() : empty_(true) {}
    callback_reference(const callback_reference &t) : empty_(t.empty_), iter_(t.iter_) {}
    bool operator==(callback_reference const& t) {
if (t.empty_ != empty_) return false;
if (t.empty_ && empty_) return true;
return (t.iter_ == iter_);
    }
  private:
    friend class detail::future_impl;
    bool empty_;
    std::list<YAMAIL_FQNS_COMPAT::function<void (void)> >::iterator iter_;
};

template<typename R> class future;
template<typename R> class promise;

namespace detail {

template<typename T>
class return_value_base {
public:
  virtual T get() const = 0;
  virtual ~return_value_base() {}
    };

template<typename T>
class return_value_real : public return_value_base<T> {
public:
  return_value_real() {}
  explicit return_value_real(const T &value)
    : value_(new T(value)) {}

  virtual T get() const {
    return *value_;
  }
  void set(T const& value) {
    value_ = compat::shared_ptr<T>(new T(value));
  }
#if __cplusplus >= 201103L
  void set(T&& value) {
    value_ = compat::shared_ptr<T>(new T(std::forward<T> (value)));
  }
#endif
  virtual ~return_value_real() {}
private:
  compat::shared_ptr<T> value_;
};

template<typename T,typename U>
class return_value_type_adaptor : public return_value_base<T> {
public:
  return_value_type_adaptor(const compat::shared_ptr<return_value_base<U> > &real_value) : value_(real_value) {}
  virtual T get() const {
    return value_->get();
  }
        virtual ~return_value_type_adaptor() {}
private:
        compat::shared_ptr<return_value_base<U> > value_;
    };

  class future_impl {
    public:
future_impl() : has_value_(false), has_exception_(false), is_canceled_(false), callbacks_in_progress_(false), is_needed_() {}
bool has_value() const {
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
  return has_value_;
}
bool has_exception() const {
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
  return has_exception_;
}
bool ready() const {
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
  return (has_value_ || has_exception_);
}
void wait() {
  set_needed();
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
  while (!has_value_ && !has_exception_)
    cond_.wait(lck);
  return;
}

template <typename Clock, typename Duration>
bool timed_wait (
  YAMAIL_FQNS_COMPAT::chrono::time_point<Clock,Duration> const& abstime) 
{
  set_needed();
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
  while (!has_value_ && !has_exception_)
    if (!cond_.wait_until(lck, abstime))
      return false; /* timeout */
  return true;
}

template <typename Rep, typename Period>
bool timed_wait (
  YAMAIL_FQNS_COMPAT::chrono::duration<Rep,Period> const& reltime) 
{
  set_needed();
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
  while (!has_value_ && !has_exception_)
    if (!cond_.wait_for(lck, reltime))
      return false; /* timeout */
  return true;
}

// Could return by-ref if set_value only called once
template <typename R>
  R get(const return_value_base<R> &value) {
    set_needed();
    YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
    while (!has_value_ && !has_exception_)
      cond_.wait(lck);
    if (has_exception_)
      YAMAIL_FQNS_COMPAT::rethrow_exception(exception_);
    return value.get();
  }

template <typename R>
  bool set_value(R const& r, return_value_real<R> &value) {
    YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
    if (has_value_ || has_exception_) return false;
    value.set(r);
    has_value_ = true;
    notify(lck);
    return true;
  }

void set_exception( const YAMAIL_FQNS_COMPAT::exception_ptr &e) {
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
  if (has_value_ || has_exception_) return;
  exception_ = e;
  has_exception_ = true;
  notify(lck);
}

#if __cplusplus >= 201103L
template <typename R>
  bool set_value(R&& r, return_value_real<R> &value) {
    YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
    if (has_value_ || has_exception_) return false;
    value.set(std::forward<R> (r));
    has_value_ = true;
    notify(lck);
    return true;
  }

void set_exception(YAMAIL_FQNS_COMPAT::exception_ptr&& e) {
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
  if (has_value_ || has_exception_) return;
  exception_ = std::move (e);
  has_exception_ = true;
  notify(lck);
}
#endif

void cancel() {
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
  if (has_value_ || has_exception_) return; // ignore 
  exception_ = YAMAIL_FQNS_COMPAT::make_exception_ptr(future_cancel());
  has_exception_ = true;
  is_canceled_ = true;
  YAMAIL_FQNS_COMPAT::function<void (void)> canhan = cancel_handler_;
  notify(lck); //unlocks mutex, also deletes cancel_handler_
  canhan(); 
}
void end_promise() {
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
  if (has_value_ || has_exception_) return; // ignore 
  exception_ = YAMAIL_FQNS_COMPAT::make_exception_ptr(broken_promise());
  has_exception_ = true;
  notify(lck);
}

callback_reference 
add_callback(const YAMAIL_FQNS_COMPAT::function<void (void)> f) {
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
        if (has_value_ || has_exception_) {
    lck.unlock(); // never call a callback within the mutex
    f(); // future already fulfilled.  Call the callback immediately.
    return callback_reference(); //return empty callback_reference
  }
  callbacks_.push_front(f);
  callback_reference cb_ref;
        cb_ref.iter_ = callbacks_.begin();
  cb_ref.empty_ = false;
        return cb_ref;
}

void remove_callback(const callback_reference &ref) {
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
        if (callbacks_in_progress_) {
    while (callbacks_in_progress_)
      cond_.wait(lck);
    //notify already removed all callbacks
          return;
        }
        if (has_value_ || has_exception_) return; //ignore, already set, and automatically removed
  callbacks_.erase(ref.iter_);
      }
bool set_cancel_handler( const YAMAIL_FQNS_COMPAT::function<void (void)> &f ) {
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
  if (is_canceled_) {
    lck.unlock();
    f();
    return false;
  }
  if (has_value_ || has_exception_ || callbacks_in_progress_) 
    return false; //ignore, future already set, cancel will never happen
  cancel_handler_ = f;
  return true;
}

compat::shared_ptr<future_impl> get_needed_future() const {
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
  if (!is_needed_) // allocate if desired
    is_needed_.reset(new future_impl);
  return is_needed_;
}

// as-needed functionality permits lazy eval and as-needed producer/consumer
void set_needed() {
  compat::shared_ptr<future_impl> n = get_needed_future();
  n->set();
}

bool is_needed() const {
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
  // if we are bound, we always say we are already needed
  return ((is_needed_ && is_needed_->ready()) || has_value_ || has_exception_);
}
void wait_until_needed() const {
  compat::shared_ptr<future_impl> n = get_needed_future();
  n->wait();
}
    private:
typedef std::list<YAMAIL_FQNS_COMPAT::function<void (void)> > func_list_t;
void notify(YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> &lck) {
  callbacks_in_progress_ = true;
  cond_.notify_all();
  func_list_t cb(callbacks_);
  lck.unlock();
  func_list_t::iterator it;
  for (it = cb.begin(); it != cb.end(); ++it)
    (*it)();
        // delete all callbacks - they will never be needed again
        // that is also why this clear is thread-safe outside the mutex
        callbacks_.clear();
  cancel_handler_ = YAMAIL_FQNS_COMPAT::function<void (void)>();
  // the below is in case someone tried to remove while we are calling
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck2(mutex_);
  callbacks_in_progress_ = false;
  cond_.notify_all();
}
bool set() { // a very simple set, used for as_needed_ future
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> lck(mutex_);
  if (has_value_ || has_exception_) return false;
  has_value_ = true;
  notify(lck);
  return true;
}
bool has_value_;
bool has_exception_;
bool is_canceled_;
YAMAIL_FQNS_COMPAT::exception_ptr exception_;
mutable YAMAIL_FQNS_COMPAT::mutex mutex_;
mutable YAMAIL_FQNS_COMPAT::condition_variable_any cond_;
func_list_t callbacks_;
bool callbacks_in_progress_;
mutable compat::shared_ptr<future_impl> is_needed_;
YAMAIL_FQNS_COMPAT::function<void (void)> cancel_handler_;
  };

  template<typename R> 
    class promise_impl {
public:
  promise_impl() : f_(new future_impl), value_(new return_value_real<R>) {};
  ~promise_impl() {
    try { f_->end_promise(); } catch (...) {}
  }
  compat::shared_ptr<detail::future_impl> f_;
  compat::shared_ptr<return_value_real<R> > value_;
    };

} // namespace detail
} // namespace future
YAMAIL_FQNS_CONCURRENCY_END
#endif // _YAMAIL_CONCURRENCY_FUTURE_FUTURE_DETAIL_HPP_

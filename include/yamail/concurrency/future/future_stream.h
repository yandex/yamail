#ifndef _YAMAIL_CONCURRENCY_FUTURE_FUTURE_FUTURE_STREAM_HPP_
#define _YAMAIL_CONCURRENCY_FUTURE_FUTURE_FUTURE_STREAM_HPP_
#include <yamail/config.h>
#include <yamail/concurrency/namespace.h>
#include <yamail/concurrency/future/future.h>

#include <yamail/compat/shared_ptr.h>
#include <yamail/compat/mutex.h>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/optional.hpp>

YAMAIL_FQNS_CONCURRENCY_BEGIN
namespace future {

class end_of_stream : public std::runtime_error {
  public:
    end_of_stream() : std::runtime_error("End of Future Stream") {}
};

template <typename T>
  class future_stream {
    protected:
class item;
    public:
class iterator;
typedef YAMAIL_FQNS_COMPAT::shared_ptr<item> item_p;
    protected:
struct item {
  item(const T &value, const promise<item_p> &p) : value_(value), next_(p) {}
  T value_;
  future<item_p> next_;
};

struct stream_impl {
  stream_impl(const promise<item_p> &p) : head_(p) {}
  future<item_p> head_; //the promise of the next element
  mutable YAMAIL_FQNS_COMPAT::mutex mutex_;
};
#if 1
struct hold_impl {
  hold_impl(const future<item_p> &itm) : item_(itm) {}
  ::boost::optional<future<item_p> > item_;
  mutable YAMAIL_FQNS_COMPAT::mutex mutex_;
};
#endif
    public:
class iterator : public ::boost::iterator_facade<
     iterator, T, ::boost::forward_traversal_tag> 
      {
public:
  iterator() : item_(promise<item_p>()) {}
  iterator(const iterator &t) : item_(t.item_) {}
  iterator(const future<item_p> &head) : item_(head) {}

  T recv() {
    if (!item_.get()) throw (end_of_stream());
    T tmp = item_.get()->value_;
    item_ = item_.get()->next_;
    return tmp;
  }

  bool ready() const {
    return item_.ready();
  }

  bool closed() const {
    if (!item_.ready()) return false;
    if (item_.has_exception()) return true;
    if (!item_.get()) return true; // closed
    return false;
  }

  void set_needed() {
    item_.set_needed();
  }

  void reset() {
    item_ = promise<item_p>(); // will set broken promise exception
  }

  future<item_p> next() {
    item_p ip = item_.get();
    if (!ip) throw (end_of_stream());
    return item_.get()->next_;
  }
private:
  void increment() { item_ = item_.get()->next_; }

  // NOTE this blocks!
  bool equal(iterator const& t) const {
    return (item_.get() == t.item_.get());
  }

  T& dereference() const { 
    if (!item_.get()) { //DEBUG!
      std::cout << "dref got eos, item_.get() = " << item_.get() << "\n";
      throw (end_of_stream());
    }
    return item_.get()->value_;
  }

  future<item_p> item_;
  friend class ::boost::iterator_core_access;
      };

#if 1
class hold {
  public:
    class hold_already_released : public std::exception {
      virtual char *what() {
  return "Attempt to release future_stream::hold more than once";
      }
    };
    hold(const hold &t) : impl_(t.impl_) {}
    hold(const future<item_p> &head) : impl_(new hold_impl(head)) {}
    iterator release() {
      YAMAIL_FQNS_COMPAT::lock_guard<YAMAIL_FQNS_COMPAT::mutex> 
          lck(impl_->mutex_);
      if (!impl_->item_) throw hold_already_released();
      iterator iter(impl_->item_.get());
      impl_->item_.reset();
      return iter;
    }
  private:
    YAMAIL_FQNS_COMPAT::shared_ptr<hold_impl> impl_;
};
#endif

future_stream(const future_stream &ps) : impl_(ps.impl_) {}

future_stream &operator=(const future_stream& t) { 
  impl_ = t.impl_;
  return *this;
}

future<item_p> head() {
  YAMAIL_FQNS_COMPAT::lock_guard<YAMAIL_FQNS_COMPAT::mutex> 
      lck(impl_->mutex_);
  return impl_->head_; //iterator(impl_->head_);
}

void reset() {
  impl_.reset();
}

#if 1
hold take_hold() {
  YAMAIL_FQNS_COMPAT::lock_guard<YAMAIL_FQNS_COMPAT::mutex> 
      lck(impl_->mutex_);
  return hold(impl_->head_);
}
#endif

iterator begin() {
  return head();
}

iterator end() {
  yplatform::future::promise<item_p> p;
  p.set(item_p());
  return iterator(p); // empty iterator
}
    protected:
future_stream() : impl_() {}
YAMAIL_FQNS_COMPAT::shared_ptr<stream_impl> impl_;
  };

template<typename T>
  class promise_stream : public future_stream<T> {
    private:
typedef future_stream<T> base_type;
typedef typename base_type::item_p item_p;
typedef typename base_type::item item;
typedef typename base_type::stream_impl stream_impl;
using base_type::impl_;
    public:
typedef typename base_type::iterator iterator;
promise_stream() 
  :  future_stream<T>(), prom_(new promise<item_p>)
{
  impl_.reset(new stream_impl(*prom_));
}

promise_stream(const promise_stream<T> &ps) 
  : future_stream<T>(ps), prom_(ps.prom_) 
{}

promise_stream &operator=(const promise_stream& t) { 
  base_type::operator=((base_type&)t);
  prom_ = t.prom_;
}

void send(const T &value) {
  YAMAIL_FQNS_COMPAT::lock_guard<YAMAIL_FQNS_COMPAT::mutex> 
      lck(impl_->mutex_);
  promise<item_p> p; //create next promise
  prom_->set(item_p(new item(value, p))); //fulfill current promise
  impl_->head_ = p; //set the head
        (*prom_) = p; //remember our next promise
}

void reset() {
  base_type::reset();
  prom_.reset();
}

void close() {
  YAMAIL_FQNS_COMPAT::lock_guard<YAMAIL_FQNS_COMPAT::mutex> 
      lck(impl_->mutex_);
  prom_->set(item_p()); //fulfill current promise with 0x0
}

void wait_until_needed() {
  YAMAIL_FQNS_COMPAT::unique_lock<YAMAIL_FQNS_COMPAT::mutex> 
      lck(impl_->mutex_);
  promise<item_p> p = *prom_;
  lck.unlock();
  p.wait_until_needed();
}

bool is_needed() {
  YAMAIL_FQNS_COMPAT::lock_guard<YAMAIL_FQNS_COMPAT::mutex> 
      lck(impl_->mutex_);
  return prom_->is_needed();
}

future<void> get_needed_future() {
  YAMAIL_FQNS_COMPAT::lock_guard<YAMAIL_FQNS_COMPAT::mutex> 
      lck(impl_->mutex_);
  return prom_->get_needed_future();
}
    private:
YAMAIL_FQNS_COMPAT::shared_ptr<promise<item_p> > prom_;
  };

}
YAMAIL_FQNS_CONCURRENCY_END
#endif // _YAMAIL_CONCURRENCY_FUTURE_FUTURE_FUTURE_STREAM_HPP_

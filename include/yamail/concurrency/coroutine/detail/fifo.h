#ifndef _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_FIFO_H_
#define _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_FIFO_H_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/namespace.h>

#include <yamail/concurrency/coroutine/detail/worker_coroutine.h>

#include <boost/noncopyable.hpp>

YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN
namespace detail {

class fifo: boost::noncopyable
{
public:
  fifo () BOOST_NOEXCEPT
    : head_ (0)
    , tail_ (0)
  {}

  bool empty () const BOOST_NOEXCEPT
  {
  	return 0 == head_; 
  }

  void push (worker_coroutine* item) BOOST_NOEXCEPT
  {
  	BOOST_ASSERT (0 != item);
  	BOOST_ASSERT (0 == item->next ());

  	if (empty ())
  		head_ = tail_ = item;
  	else
    {
    	tail_->next (item);
    	tail_ = item;
    }
  }

  worker_coroutine* pop () BOOST_NOEXCEPT
  {
  	BOOST_ASSERT (! empty ());

  	worker_coroutine* item = head_;
  	head_ = head_->next ();
  	if (0 == head_) tail_ = 0;
  	item->next_reset ();
  	return item;
  }

  void swap (fifo& other)
  {
  	std::swap (head_, other.head_);
  	std::swap (tail_, other.tail_);
  }

private:
  worker_coroutine* head_;
  worker_coroutine* tail_;
};

} // namespace detail
YAMAIL_FQNS_CONCURRENCY_COROUTINE_END
#endif // _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_FIFO_H_

#ifndef _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_WAITING_QUEUE_H_
#define _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_WAITING_QUEUE_H_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/namespace.h>

#include <yamail/concurrency/coroutine/detail/worker_coroutine.h>

#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>

YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN
namespace detail {

class waiting_queue: boost::noncopyable
{
public:
  waiting_queue () BOOST_NOEXCEPT
    : head_ (0)
    , tail_ (0)
  {
  }

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
    	worker_coroutine* c = head_, *prev = 0;

    	do {
    		worker_coroutine* next = c->next ();
    		if (item->time_point () <= c->time_point ())
        {
        	if (head_ == c)
          {
          	BOOST_ASSERT (0 == prev);

          	item->next (c);
          	head_ = item;
          }
          else
          {
          	BOOST_ASSERT (0 != prev);

          	item->next (c);
          	prev->next (item);
          }
          break;
        }
        else if (tail_ == c)
        {
        	BOOST_ASSERT (0 == next);

        	tail_->next (item);
        	tail_ = item;
        	break;
        }

        prev = c;
        c = next;

      } while (0 != c);
    }
  }

  worker_coroutine* top () const BOOST_NOEXCEPT
  {
  	BOOST_ASSERT (! empty ());
  	return head_;
  }

  template <typename SchedAlgo, typename Fn>
  void move_to (SchedAlgo* sched_algo, Fn fn)
  {
  	BOOST_ASSERT (sched_algo);

  	worker_coroutine* c = head_, *prev = 0;
  	while (0 != c)
    {
    	worker_coroutine* next = c->next ();
    	if (fn (c))
    	{
    		if (c == head_)
        {
        	BOOST_ASSERT (0 == prev);

        	head_ = next;
        	if (0 == head_)
        		tail_ = 0;
        }
        else
        {
        	BOOST_ASSERT (0 != prev);

        	if (0 == next)
        		tail_ = prev;

        	prev->next (next);
        }

        c->next_reset ();
        c->time_point_reset ();
        sched_algo->awakened (c);
      }
      else
      	prev = c;

      c = next;
    }
  }

  void swap (waiting_queue& other)
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
#endif // _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_WAITING_QUEUE_H_

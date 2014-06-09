#ifndef _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_CSS_H_
#define _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_CSS_H_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/namespace.h>

#include <yamail/compat/thread.h>

#include <boost/intrusive_ptr.hpp>

YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN
namespace detail {

class css_cleanup_function
{
public:
  typedef boost::intrusive_ptr<css_cleanup_function> ptr_t;

  css_cleanup_function () : use_count_ (0) {}
  virtual ~css_cleanup_function () {}

  virtual void operator() (void* data) = 0;

  friend inline void 
  intrusive_ptr_add_ref (css_cleanup_function* p) BOOST_NOEXCEPT
  {
  	++p->use_count_;
  }
  
  friend inline void 
  intrusive_ptr_release (css_cleanup_function* p) 
  {
  	if (0 == --p->use_count_) 
  		delete p;
  }

private:
	compat::atomic<std::size_t> use_count_;
};

} // namespace detail
YAMAIL_FQNS_CONCURRENCY_COROUTINE_END
#endif // _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_CSS_H_

#ifndef _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_COROUTINE_BASE_H_
#define _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_COROUTINE_BASE_H_
#include <yamail/config.h>
#include <yamail/concurrency/coroutine/namespace.h>

#include <boost/noncopyable.hpp>
#include <istream>

YAMAIL_FQNS_CONCURRENCY_COROUTINE_BEGIN
namespace detail {

struct coroutine_base : private boost::noncopyable
{
	class id;

	coroutine_base () {}
	virtual ~coroutine_base () {}
	virtual bool is_ready () const BOOST_NOEXCEPT = 0;
	virtual void set_ready () const BOOST_NOEXCEPT = 0;
	virtual id get_id () const BOOST_NOEXCEPT = 0;
};

class coroutine_base::id
{
	coroutine_base* impl_;

public:
           id ()                     BOOST_NOEXCEPT : impl_ (0)    {}
  explicit id (coroutine_base* impl) BOOST_NOEXCEPT : impl_ (impl) {}

  operator bool () const BOOST_NOEXCEPT
  {
  	return 0 != impl_;
  }

  bool operator! () const BOOST_NOEXCEPT
  {
  	return 0 == impl_;
  }

private:
  friend bool operator== (id const& a, id const& b) BOOST_NOEXCEPT
  {
  	return a.impl_ == b.impl_;
  }

  friend bool operator!= (id const& a, id const& b) BOOST_NOEXCEPT
  {
  	return a.impl_ != b.impl_;
  }

  friend bool operator< (id const& a, id const& b) BOOST_NOEXCEPT
  {
  	return a.impl_ < b.impl_;
  }

  friend bool operator> (id const& a, id const& b) BOOST_NOEXCEPT
  {
  	return a.impl_ > b.impl_;
  }

  friend bool operator<= (id const& a, id const& b) BOOST_NOEXCEPT
  {
  	return ! (a > b);
  }

  friend bool operator>= (id const& a, id const& b) BOOST_NOEXCEPT
  {
  	return ! (a < b);
  }

  template <typename CharT, typename TraitsT>
  friend std::basic_ostream<CharT, TraitsT>&
  operator<< (std::basic_ostream<CharT,TraitsT>& os, id const& x)
  {
  	if (0 != x.impl_) return os << x.impl_;
  	else              return os << "{not-valid}";
  }
};

} // namespace detail
YAMAIL_FQNS_CONCURRENCY_COROUTINE_END
#endif // _YAMAIL_CONCURRENCY_COROUTINE_DETAIL_COROUTINE_BASE_H_

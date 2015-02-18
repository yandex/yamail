#ifndef _YAMAIL_UTILITY_CAPTURE_H_
#define _YAMAIL_UTILITY_CAPTURE_H_
#include <yamail/config.h>

#if YAMAIL_CPP < 11

# error This file requires compiler and library support for the \
ISO C++ 2011 standard. This support is currently experimental, and must be \
enabled with the -std=c++11 or -std=gnu++11 compiler options.

#else

#include <yamail/utility/namespace.h>

#include <yamail/utility/apply.h>

#include <utility>
#include <tuple>
#include <type_traits>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace utility {
#else
YAMAIL_FQNS_UTILITY_BEGIN
#endif // GENERATING_DOCUMENTATION

// Usage: 
//  std::unique_ptr<int> p {...}, q {...};
//  auto lambda = capture (
//    [] (std::unique_ptr<int>& p, std::unique_ptr<int>& q) 
//    { return std::move (p); },
//    std::move (p), std::move (q));

namespace detail {

template <typename F, typename... T>
class capture_impl
{
	template <class U> struct transform;
	template <class... U> struct transform <std::tuple<U...> >
	{
		typedef std::tuple<typename std::decay<U>::type ...> type;
  };

	typename std::decay<F>::type f;
	typename transform<std::tuple<T...> >::type t;

public:
  capture_impl (F&& f, T&& ...t)
    : f ( std::forward<F> (f) )
    , t ( std::forward_as_tuple (std::forward<T> (t)...) )
  {
  }

  template <typename ...Ts> auto operator() (Ts&& ...args)
      -> decltype (apply (f, t, std::forward<Ts> (args)...))
  {
  	return apply (f, t, std::forward<Ts> (args)...);
  }

#if YAMAIL_CPP < 14 // XXX probably need some more clever workaround here.
  template <typename ...Ts> auto operator() (Ts&& ...args) const
      -> decltype (apply (f, t, std::forward<Ts> (args)...))
  {
  	return apply (f, t, std::forward<Ts> (args)...);
  }
#endif
};

}

/// move lambda capture emulation for c++11 
template <typename F, typename T0, typename ...Ts>
detail::capture_impl<F,T0,Ts...> // at least two arguments required
capture (F&& f, T0&& t0, Ts&& ...ts)
{
	return detail::capture_impl<F,T0,Ts...> (std::forward<F> (f), 
	    std::forward<T0> (t0), std::forward<Ts> (ts)...);
}

#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_UTILITY_END
#endif // GENERATING_DOCUMENTATION

#endif // C++11

#endif // _YAMAIL_UTILITY_CAPTURE_H_

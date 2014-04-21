#ifndef _YAMAIL_UTILITY_CAPTURE_H_
#define _YAMAIL_UTILITY_CAPTURE_H_
#include <yamail/config.h>
#include <yamail/utility/namespace.h>

#include <yamail/utility/apply.h>

#include <utility>
#include <tuple>
#include <type_traits>

YAMAIL_NS_BEGIN
YAMAIL_NS_UTILITY_BEGIN

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

	typename transform<std::tuple<T...> >::type t;
	F f; // Should we copy "f" also?

public:
  capture_impl (F&& f, T&& ...t)
    : f ( std::forward<F> (f) )
    , t ( std::forward_as_tuple (t...) )
  {}

  template <typename ...Ts> auto operator() (Ts&& ...args)
      -> decltype (apply (f, t, std::forward<Ts> (args)...))
  {
  	return apply (f, t, std::forward<Ts> (args)...);
  }

  template <typename ...Ts> auto operator() (Ts&& ...args) const
      -> decltype (apply (f, t, std::forward<Ts> (args)...))
  {
  	return apply (f, t, std::forward<Ts> (args)...);
  }
};

}

/// move lambda capture emulation for c++11 
template <typename F, typename T0, typename ...Ts>
detail::capture_impl<F,T0,Ts...> 
capture (F&& f, T0&& t0, Ts&& ...ts)
{
	return detail::capture_impl<F,T0,Ts...> (std::forward<F> (f), 
	    std::forward<T0> (t0), std::forward<Ts> (ts)...);
}

YAMAIL_NS_UTILITY_END
YAMAIL_NS_END

#endif // _YAMAIL_UTILITY_CAPTURE_H_

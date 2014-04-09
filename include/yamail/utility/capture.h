#ifndef _YAMAIL_UTILITY_CAPTURE_H_
#define _YAMAIL_UTILITY_CAPTURE_H_
#include <yamail/config.h>
#include <yamail/utility/namespace.h>

#include <utility>

YAMAIL_NS_BEGIN
YAMAIL_NS_UTILITY_BEGIN

// Usage: 
//  std::unique_ptr<int> p {...};
//  auto lambda = capture (std::move (p), 
//    [] (std::unique_ptr<int>& p) { return std::move (p); });

template <typename F, typename F>
class capture_impl
{
	T x;
	F f;
public:
  capture_impl (T&& x, F&& f)
    : x { std::forward<T> (x) }
    , f { std::forward<F> (f) }
  {}

  template <typename ...Ts> auto operator() (Ts&& ...args)
      -> decltype (f (x, std::forward<Ts> (args)...))
  {
  	return f (x, std::forward<Ts> (args)...));
  }

  template <typename ...Ts> auto operator() (Ts&& ...args) const
      -> decltype (f (x, std::forward<Ts> (args)...))
  {
  	return f (x, std::forward<Ts> (args)...));
  }
};

/// move lambda capture emulation for c++11 
template <typename T, typename F>
capture_impl<T,F> capture (T&& x, F&& f)
{
	return capture_impl<T,F> (std::forward<T> (x), std::forward<F> (f));
}

YAMAIL_NS_UTILITY_END
YAMAIL_NS_END

#endif // _YAMAIL_UTILITY_CAPTURE_H_

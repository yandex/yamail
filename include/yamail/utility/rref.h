#ifndef _YAMAIL_UTILITY_RREF_H_
#define _YAMAIL_UTILITY_RREF_H_
#include <yamail/config.h>
#include <yamail/utility/namespace.h>

#include <utility>
#include <cassert>
#include <memory>

YAMAIL_NS_BEGIN
YAMAIL_NS_UTILITY_BEGIN

/// std::rref emulation for c++11.
/// Used to emulate move lambda capture.

// Usage:
//  std::unique_ptr<int> p{new int(0)};
//  auto rref = make_rref( std::move(p) );
//  auto lambda =
//    [rref]() mutable -> std::unique_ptr<int> { return rref.move(); };

template <typename T>
struct rref_impl 
{
	rref_impl () = delete;
	rref_impl (T&& x) : x { std::move (x) } {}
	rref_impl (rref_impl& other)
	  : x { std::move (other.x) }
	  , is_copied { true }
	{
		assert (other.is_copied == false);
	}

	rref_impl (rref_impl&& other)
	  : x { std::move (other.x) }
	  , is_copied { std::move (other.is_copied }
	{
	}

	rref_impl& operator= (rref_impl other) = delete;

	T&& move () { return std::move (x); }

private:
  T x;
  bool is_copied = false;
};

template <typename T> rref_impl<T> make_rref (T&& x)
{
	return rref_impl<T> { std::move (x) };
}

YAMAIL_NS_UTILITY_END
YAMAIL_NS_END

#endif // _YAMAIL_UTILITY_RREF_H_

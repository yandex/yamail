#ifndef _YAMAIL_UTILITY_RREF_H_
#define _YAMAIL_UTILITY_RREF_H_
#include <yamail/config.h>
#include <yamail/utility/namespace.h>

#include <utility>
#include <cassert>
#include <memory>

#if YAMAIL_CPP < 11

# error This file requires compiler and library support for the \
ISO C++ 2011 standard. This support is currently experimental, and must be \
enabled with the -std=c++11 or -std=gnu++11 compiler options.

#else

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace utility {
#else
YAMAIL_FQNS_UTILITY_BEGIN
#endif // GENERATING_DOCUMENTATION

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
	  : x (std::move (other.x))
	  , is_copied (true)
	{
		assert (other.is_copied == false);
	}

	rref_impl (rref_impl&& other)
	  : x (std::move (other.x))
	  , is_copied (std::move (other.is_copied))
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


#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_UTILITY_END
#endif // GENERATING_DOCUMENTATION

#endif // c++11

#endif // _YAMAIL_UTILITY_RREF_H_

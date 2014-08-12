#ifndef _YAMAIL_UTILITY_IN_H_
#define _YAMAIL_UTILITY_IN_H_
#include <yamail/config.h>

#if YAMAIL_CPP < 11

# error This file requires compiler and library support for the \
ISO C++ 2011 standard. This support is currently experimental, and must be \
enabled with the -std=c++11 or -std=gnu++11 compiler options.

#else

#include <yamail/utility/namespace.h>

#include <utility>
#include <type_traits>
#include <cassert>

#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_UTILITY_BEGIN
#endif // GENERATING_DOCUMENTATION

template <typename T> struct in_common;
template <typename T> struct is_in_common
{ static const bool value = false;};
template <typename T> struct is_in_common<in_common<T> > 
{ static const bool value = true;};

template <typename T> struct in_common
{
  // Support for implicit conversion via perfect forwarding.
  //
  struct storage
  {
    storage (): created (false) {}
    ~storage () {if (created) reinterpret_cast<T*> (&data)->~T ();}

    bool created;
    typename std::aligned_storage<sizeof (T), alignof (T)>::type data;
  };

  /// Creates copy of data in temp storage.
  in_common (const T& l, storage&& s)
    : v_ (*new (&s.data) T (l))
    , rv_ (true) 
  {
    s.created = true;
  }

  in_common (const T& l, bool rv) : v_ (l), rv_ (rv) {}

  template <typename T1,
            typename std::enable_if<
              std::is_convertible<T1, T>::value &&
              !is_in_common<typename std::remove_reference<T1>::type>::value,
              int>::type = 0>
  in_common (T1&& x, storage&& s)
    : v_ (*new (&s.data) T (x))
    , rv_ (true) 
  {
    s.created = true;
  }

  // Accessors.
  //
  bool lvalue () const {return !rv_;}
  bool rvalue () const {return rv_;}

  operator const T& () const {return v_;}
  const T& get () const {return v_;}
  T&& rget () const {return std::move (const_cast<T&> (v_));}

  T const* operator-> () const { return &v_; }

  // Return a copy if lvalue.
  //
  T move () const
  {
    // It would have been more natural to use ?: operator, however,
    // it doesn't work as one would expected in all the compilers.
    // For example, Clang 3.0 will always call a copy constructor.
    //
    if (rv_)
      return rget ();
    else
      return v_;
  }

protected:
  const T& v_;
  bool rv_;
};

////////////////////////////////////////////////////////////////////////////////
// in
template <typename T> struct in;
template <typename T> struct is_in         { static const bool value = false;};
template <typename T> struct is_in<in<T> > { static const bool value = true;};

template <typename T> 
struct in: public in_common<T>
{
	typedef typename in::storage storage;

  in (const T& l): in_common<T> (l, false) {}
  in (T&& r): in_common<T> (r, true) {}

  template <typename T1,
            typename std::enable_if<
              std::is_convertible<T1, T>::value &&
              !is_in<typename std::remove_reference<T1>::type>::value,
              int>::type = 0>
  in (T1&& x, storage&& s = storage ())
    : in_common<T> (std::forward<T1> (x), std::move (s)) {}

  in (T& l): in_common<T> (l, false) {} // For T1&& becoming T1&.
};

////////////////////////////////////////////////////////////////////////////////
// rw_in
template <typename T> struct rw_in;
template <typename T> struct is_rw_in            { static const bool value = false;};
template <typename T> struct is_rw_in<rw_in<T> > { static const bool value = true;};

template <typename T> 
struct rw_in : public in_common<T>
{
	typedef typename rw_in::storage storage;

  rw_in (const T& l, storage&& s = storage ()) 
    : in_common<T> (l, std::move (s)) {};

  rw_in (T&& r): in_common<T> (r, true) {}

  template <typename T1,
            typename std::enable_if<
              std::is_convertible<T1, T>::value &&
              !is_rw_in<typename std::remove_reference<T1>::type>::value,
              int>::type = 0>
  rw_in (T1&& x, storage&& s = storage ()) 
    : in_common<T> (std::forward<T1> (x), std::move (s)) 
  {}

  rw_in (T& l): in_common<T> (l, false) {} // For T1&& becoming T1&.
};

////////////////////////////////////////////////////////////////////////////////
// rval_in
template <typename T> struct rval_in;

template <typename T> struct is_rval_in            
{ static const bool value = false;};

template <typename T> struct is_rval_in<rval_in<T> > 
{ static const bool value = true;};

template <typename T> 
struct rval_in : public in_common<T>
{
	typedef typename rval_in::storage storage;

  rval_in (const T& l, storage&& s = storage ()) 
    : in_common<T> (l, std::move (s)) {}

  rval_in (T&& r): in_common<T> (r, true) {}

  template <typename T1,
            typename std::enable_if<
              std::is_convertible<T1, T>::value &&
              !is_rval_in<typename std::remove_reference<T1>::type>::value,
              int>::type = 0>
  rval_in (T1&& x, storage&& s = storage ()) 
    : in_common<T> (std::forward<T1> (x), std::move (s)) {}

  rval_in (T& l, storage&& s = storage ()) 
    : in_common<T> (l, std::move (s)) {}
};


#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_UTILITY_END
#endif // GENERATING_DOCUMENTATION

#endif // C++11
#endif // _YAMAIL_UTILITY_IN_H_

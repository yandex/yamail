#ifndef _YAMAIL_UTILITY_IN_H_
#define _YAMAIL_UTILITY_IN_H_
#include <yamail/config.h>
#include <yamail/utility/namespace.h>

#include <utility>
#include <type_traits>
#include <cassert>

YAMAIL_NS_BEGIN
YAMAIL_NS_UTILITY_BEGIN

template <typename T> struct in;
template <typename T> struct is_in         { static const bool value = false;};
template <typename T> struct is_in<in<T> > { static const bool value = true;};

template <typename T> struct in
{
  in (const T& l): v_ (l), rv_ (false) {}
  in (T&& r): v_ (r), rv_ (true) {}

  // Support for implicit conversion via perfect forwarding.
  //
  struct storage
  {
    storage (): created (false) {}
    ~storage () {if (created) reinterpret_cast<T*> (&data)->~T ();}

    bool created;
    typename std::aligned_storage<sizeof (T), alignof (T)>::type data;
  };

  template <typename T1,
            typename std::enable_if<
              std::is_convertible<T1, T>::value &&
              !is_in<typename std::remove_reference<T1>::type>::value,
              int>::type = 0>
  in (T1&& x, storage&& s = storage ())
      : v_ (*new (&s.data) T (x)), rv_ (true) {s.created = true;}

  in (T& l): v_ (l), rv_ (false) {} // For T1&& becoming T1&.

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

private:
  const T& v_;
  bool rv_;
};

template <typename T> struct rw_in;
template <typename T> struct is_rw_in            { static const bool value = false;};
template <typename T> struct is_rw_in<rw_in<T> > { static const bool value = true;};

template <typename T> 
struct rw_in
{
  // Support for implicit conversion via perfect forwarding.
  //
  struct storage
  {
    storage (): created (false), data () {}
    ~storage () {if (created) reinterpret_cast<T*> (&data)->~T ();}

    bool created;
    typename std::aligned_storage<sizeof (T), alignof (T)>::type data;
  };

  rw_in (const T& l, storage&& s = storage ())
    : v_ (*new (&s.data) T (l))
    , rv_ (true) 
  {
    s.created = true;
  }

  rw_in (T&& r): v_ (r), rv_ (true) {}

  template <typename T1,
            typename std::enable_if<
              std::is_convertible<T1, T>::value &&
              !is_rw_in<typename std::remove_reference<T1>::type>::value,
              int>::type = 0>
  rw_in (T1&& x, storage&& s = storage ())
      : v_ (*new (&s.data) T (x)), rv_ (true) {s.created = true;}

  rw_in (T& l): v_ (l), rv_ (false) {} // For T1&& becoming T1&.

  // Accessors.
  //
  bool lvalue () const {return !rv_;}
  bool rvalue () const {return rv_;}

  operator T& () const {return v_;}

  T& get () const {return v_;}
  T&& rget () const {return std::move (const_cast<T&> (v_));}

  T* operator-> () const { return &v_; }

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

private:
  T& v_;
  bool rv_;
};
YAMAIL_NS_UTILITY_END
YAMAIL_NS_END

#endif // _YAMAIL_UTILITY_IN_H_

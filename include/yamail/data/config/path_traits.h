#ifndef _YAMAIL_DATA_CONFIG_PATH_TRAITS_H_
#define _YAMAIL_DATA_CONFIG_PATH_TRAITS_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <type_traits>
#include <cwchar>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <iterator>
#include <locale>

#include <boost/assert.hpp>

YAMAIL_FQNS_DATA_CP_BEGIN
namespace path_traits {

typedef std::codecvt<wchar_t, char, std::mbstate_t> codecvt_type;

template <typename T>
struct is_pathable { static const bool value = false; };

template <std::size_t N> struct is_pathable<char[N]>
  { static const bool value = true; };

template <std::size_t N> struct is_pathable<wchar_t[N]>
  { static const bool value = true; };

template <> struct is_pathable<char*> 
  { static const bool value = true; };

template <> struct is_pathable<char const*> 
  { static const bool value = true; };

template <> struct is_pathable<wchar_t*> 
  { static const bool value = true; };

template <> struct is_pathable<wchar_t const*> 
  { static const bool value = true; };

template <> struct is_pathable<std::string> 
  { static const bool value = true; };

template <> struct is_pathable<std::wstring> 
  { static const bool value = true; };

template <> struct is_pathable<std::vector<char> > 
  { static const bool value = true; };

template <> struct is_pathable<std::vector<wchar_t> > 
  { static const bool value = true; };

template <> struct is_pathable<std::list<char> > 
  { static const bool value = true; };

template <> struct is_pathable<std::list<wchar_t> > 
  { static const bool value = true; };

template <> struct is_pathable<std::deque<char> > 
  { static const bool value = true; };

template <> struct is_pathable<std::deque<wchar_t> > 
  { static const bool value = true; };

////////////////////////////////////////////////////////////////////////////////
// empty

template <typename Container> 
inline 
typename std::enable_if<! std::is_array<Container>::value, bool>::type
empty (Container const& c) { return c.begin () == c.end (); }

template <typename T> 
inline bool
empty (T* const& c_str)
{
  BOOST_ASSERT (c_str);
  return ! *c_str;
}

template <typename T, size_t N>
bool empty (T (&x) [N])
{
  return ! x[0];
}

////////////////////////////////////////////////////////////////////////////////
// convert

void convert (const char* from, const char* from_end, 
    std::wstring& to, codecvt_type const& cvt);

void convert (const wchar_t* from, const wchar_t* from_end, 
    std::string& to, codecvt_type const& cvt);

inline void convert (char const* from, std::wstring& to, 
    codecvt_type const& cvt)
{
  BOOST_ASSERT (from);
  convert (from, 0, to, cvt);
}

inline void convert (wchar_t const* from, std::string& to, 
    codecvt_type const& cvt)
{
  BOOST_ASSERT (from);
  convert (from, 0, to, cvt);
}

// char

inline void
convert (char const* from, char const* from_end, std::string& to, 
    codecvt_type const&)
{
  BOOST_ASSERT (from);
  BOOST_ASSERT (from_end);
  to.append (from, from_end);
}

inline void
convert (char const* from, std::string& to, codecvt_type const&)
{
  BOOST_ASSERT (from);
  to += from;
}


// wchar_t

inline void
convert (wchar_t const* from, wchar_t const* from_end, std::wstring& to, 
    codecvt_type const&)
{
  BOOST_ASSERT (from);
  BOOST_ASSERT (from_end);
  to.append (from, from_end);
}

inline void
convert (wchar_t const* from, std::wstring& to, codecvt_type const&)
{
  BOOST_ASSERT (from);
  to += from;
}

////////////////////////////////////////////////////////////////////////////////
// Source dispatch.

// contiguous containers

template <class U>
inline void
dispatch (std::string const& c, U& to, codecvt_type const& cvt)
{
  if (c.size ())
    convert (&*c.begin (), &*c.begin () + c.size (), to, cvt);
}

template <class U>
inline void
dispatch (std::wstring const& c, U& to, codecvt_type const& cvt)
{
  if (c.size ())
    convert (&*c.begin (), &*c.begin () + c.size (), to, cvt);
}

template <class U>
inline void
dispatch (std::vector<char> const& c, U& to, codecvt_type const& cvt)
{
  if (c.size ())
    convert (&*c.begin (), &*c.begin () + c.size (), to, cvt);
}

template <class U>
inline void
dispatch (std::vector<wchar_t> const& c, U& to, codecvt_type const& cvt)
{
  if (c.size ())
    convert (&*c.begin (), &*c.begin () + c.size (), to, cvt);
}

//  non-contiguous containers

template <class Container, class U>
inline 
typename std::enable_if<! std::is_array<Container>::value, void>::type
dispatch (Container const& c, U& to, codecvt_type const& cvt)
{
  if (c.size ())
  {
    std::basic_string<typename Container::value_type> s (c.begin (), c.end ());
    convert (s.c_str (), s.c_str () + s.size (), to, cvt);
  }
}

template <class T, class U> 
inline void
dispatch (T* const& c_str, U& to, codecvt_type& cvt)
{
  BOOST_ASSERT (c_str);
  convert (c_str, to, cvt);
}

// C-style arrays dispatch?

} // namespace path_traits
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_PATH_TRAITS_H_

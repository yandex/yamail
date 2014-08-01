#ifndef _YAMAIL_UTILITY_STORAGE_H_
#define _YAMAIL_UTILITY_STORAGE_H_
#include <yamail/config.h>
#include <yamail/utility/namespace.h>

#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace utility {
#else
YAMAIL_FQNS_UTILITY_BEGIN
#endif // GENERATING_DOCUMENTATION

/// Creates aligned storage for temporary values.
template <typename T>
struct storage 
{
  bool created;

  typename boost::aligned_storage<
      sizeof (T)
    , boost::alignment_of<T>::value
  >::type data;

  _constexpr storage () _noexcept : created (false) {}
  ~storage ()
  {
  	if (created)
  		reinterpret_cast<T*> (&data)->~T ();
  }
}; // struct storage

#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::utility
#else
YAMAIL_FQNS_UTILITY_END
#endif // GENERATING_DOCUMENTATION


#endif // _YAMAIL_UTILITY_STORAGE_H_

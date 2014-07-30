#ifndef _YAMAIL_COMPAT_MAKE_UNIQUE_H_
#define _YAMAIL_COMPAT_MAKE_UNIQUE_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

#if defined(HAVE_STD_MAKE_UNIQUE) && HAVE_STD_MAKE_UNIQUE
# include <memory>
#else 
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#endif // HAVE_STD_MAKE_UNIQUE

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace compat {
#else
YAMAIL_FQNS_COMPAT_BEGIN
#endif // GENERATING_DOCUMENTATION

#if defined(HAVE_STD_MAKE_UNIQUE) && HAVE_STD_MAKE_UNIQUE
using std::make_unique;
#else
template<class T> struct _Unique_if {
    typedef std::unique_ptr<T> _Single_object;
};

template<class T> struct _Unique_if<T[]> {
    typedef std::unique_ptr<T[]> _Unknown_bound;
};

template<class T, std::size_t N> struct _Unique_if<T[N]> {
    typedef void _Known_bound;
};

template<class T, class... Args>
    typename _Unique_if<T>::_Single_object
    make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

template<class T>
    typename _Unique_if<T>::_Unknown_bound
    make_unique(std::size_t n) {
        typedef typename std::remove_extent<T>::type U;
        return std::unique_ptr<T>(new U[n]());
    }

template<class T, class... Args>
    typename _Unique_if<T>::_Known_bound
    make_unique(Args&&...) = delete;
#endif

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace compat {
#else
YAMAIL_FQNS_COMPAT_END 
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_COMPAT_MAKE_UNIQUE_H_

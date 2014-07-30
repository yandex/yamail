#ifndef _YAMAIL_COMPAT_EXCEPTION_H_
#define _YAMAIL_COMPAT_EXCEPTION_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

#if defined(HAVE_STD_EXCEPTION) && HAVE_STD_EXCEPTION
# include <exception>
#else 
# include <boost/exception_ptr.hpp>
# include <boost/exception/exception.hpp>
#endif // HAVE_STD_EXCEPTION

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace compat {
#else
YAMAIL_FQNS_COMPAT_BEGIN
#endif // GENERATING_DOCUMENTATION

#if defined(HAVE_STD_EXCEPTION) && HAVE_STD_EXCEPTION
using std::exception_ptr;
using std::rethrow_exception;
using std::current_exception;
using std::make_exception_ptr;

template <typename E> void throw_exception (E const& e) { throw e; }
#else
using boost::exception_ptr;
using boost::rethrow_exception;
using boost::current_exception;
using boost::throw_exception;

template <class E> 
inline exception_ptr 
make_exception_ptr (E const& e) _noexcept 
{
#if 1
	return boost::copy_exception (e);
#else
  try { throw e; }
  catch (...) { return current_exception (); }
#endif
}
#endif

#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_COMPAT_END 
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_COMPAT_EXCEPTION_H_

#ifndef _YAMAIL_COMPAT_SYSTEM_H_
#define _YAMAIL_COMPAT_SYSTEM_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

#if defined(HAVE_STD_SYSTEM) && HAVE_STD_SYSTEM
# include <system_error>
#else 
# include <boost/system/error_code.hpp>
# include <boost/system/system_error.hpp>
#endif // HAVE_STD_SYSTEM

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace compat {
#else
YAMAIL_FQNS_COMPAT_BEGIN
#endif // GENERATING_DOCUMENTATION

#if defined(HAVE_STD_SYSTEM) && HAVE_STD_SYSTEM
using std::system_error;
using std::error_code;
using std::error_condition;
using std::error_category;
using std::make_error_code;
using std::make_error_condition;
using std::generic_category;
using std::system_category;
using std::errc;

using std::is_error_code_enum;
using std::is_error_condition_enum;
#else
using boost::system::system_error;
using boost::system::error_code;
using boost::system::error_condition;
using boost::system::error_category;
using boost::system::errc::make_error_code;
using boost::system::errc::make_error_condition;
using boost::system::generic_category;
using boost::system::system_category;
namespace errc = ::boost::system::errc;

using ::boost::system::is_error_code_enum;
using ::boost::system::is_error_condition_enum;
#endif

#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_COMPAT_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_COMPAT_SYSTEM_H_

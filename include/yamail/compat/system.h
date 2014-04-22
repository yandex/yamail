#ifndef _YAMAIL_COMPAT_SYSTEM_H_
#define _YAMAIL_COMPAT_SYSTEM_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

#if defined(HAVE_STD_SYSTEM)
# include <system_error>
#else 
# include <boost/error_code.hpp>
# include <boost/system_error.hpp>
#endif // HAVE_STD_SYSTEM

YAMAIL_FQNS_COMPAT_BEGIN

#if defined(HAVE_STD_SYSTEM)
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
using boost::system_error;
using boost::error_code;
using boost::error_condition;
using boost::error_category;
using boost::make_error_code;
using boost::make_error_condition;
using boost::generic_category;
using boost::system_category;
using boost::errc;

using boost::is_error_code_enum;
using boost::is_error_condition_enum;
#endif

YAMAIL_FQNS_COMPAT_END 

#endif // _YAMAIL_COMPAT_SYSTEM_H_

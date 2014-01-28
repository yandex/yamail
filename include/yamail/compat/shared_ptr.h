#ifndef _YAMAIL_COMPAT_SHARED_PTR_H_
#define _YAMAIL_COMPAT_SHARED_PTR_H_
#include <yamail/config.h>
#include <yamail/compat/namespace.h>

#if defined(HAVE_STD_SHARED_PTR)
# include <memory>
#else 
# include <boost/shared_ptr.hpp>
# include <boost/make_shared.hpp>
# include <boost/weak_ptr.hpp>
#endif // HAVE_STD_SHARED_PTR_H_

YAMAIL_NS_BEGIN 
YAMAIL_NS_COMPAT_BEGIN

#if defined(HAVE_STD_SHARED_PTR)
using std::shared_ptr;
using std::weak_ptr;
using std::make_shared;
using std::allocate_shared;
using std::enable_shared_from_this;
using std::static_pointer_cast;
using std::const_pointer_cast;
using std::dynamic_pointer_cast;
#else
using boost::shared_ptr;
using boost::weak_ptr;
using boost::make_shared;
using boost::allocate_shared;
using boost::enable_shared_from_this;
using boost::static_pointer_cast;
using boost::const_pointer_cast;
using boost::dynamic_pointer_cast;
#endif

YAMAIL_NS_COMPAT_END 
YAMAIL_NS_END
#endif // _YAMAIL_COMPAT_SHARED_PTR_H_

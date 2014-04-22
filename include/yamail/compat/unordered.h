#ifndef _YAMAIL_COMPAT_UNORDERED_H_
#define _YAMAIL_COMPAT_UNORDERED_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

#if defined(HAVE_STD_UNORDERED)
# include <unordered_map>
# include <unordered_set>
#else 
# include <boost/unordered_map.hpp>
# include <boost/unordered_set.hpp>
#endif // HAVE_STD_UNORDERED

YAMAIL_NS_BEGIN 
YAMAIL_NS_COMPAT_BEGIN

#if defined(HAVE_STD_UNORDERED)
using std::unordered_map;
using std::unordered_multimap;
using std::unordered_set;
using std::unordered_multiset;
#else
using boost::unordered_map;
using boost::unordered_multimap;
using boost::unordered_set;
using boost::unordered_multiset;
#endif

YAMAIL_NS_COMPAT_END 
YAMAIL_NS_END
#endif // _YAMAIL_COMPAT_UNORDERED_H_

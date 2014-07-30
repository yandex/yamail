#ifndef _YAMAIL_COMPAT_UNORDERED_H_
#define _YAMAIL_COMPAT_UNORDERED_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

#if defined(HAVE_STD_UNORDERED) && HAVE_STD_UNORDERED
# include <unordered_map>
# include <unordered_set>
#else 
# include <boost/unordered_map.hpp>
# include <boost/unordered_set.hpp>
#endif // HAVE_STD_UNORDERED

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace compat {
#else
YAMAIL_FQNS_COMPAT_BEGIN
#endif // GENERATING_DOCUMENTATION

#if defined(HAVE_STD_UNORDERED) && HAVE_STD_UNORDERED
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

#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_COMPAT_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_COMPAT_UNORDERED_H_

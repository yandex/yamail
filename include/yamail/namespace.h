#ifndef _YAMAIL_NAMESPACE_H_
#define _YAMAIL_NAMESPACE_H_

#include <yamail/config.h>
#include <yamail/version.h>

#define YAMAIL_NS yamail_100000
#define YAMAIL_FQNS ::YAMAIL_NS

#define YAMAIL_NS_BEGIN namespace YAMAIL_NS {
#define YAMAIL_NS_END }

#define YAMAIL_FQNS_BEGIN YAMAIL_NS_BEGIN
#define YAMAIL_FQNS_END YAMAIL_NS_END

#define YAMAIL_NS_ALIAS yamail
#define YAMAIL_NS_ALIAS_SHORT y
#define YAMAIL_FQNS_ALIAS ::YAMAIL_NS_ALIAS

YAMAIL_NS_BEGIN 
YAMAIL_NS_END

// namespace YAMAIL_NS_ALIAS = YAMAIL_FQNS;
// namespace YAMAIL_NS_ALIAS_SHORT = YAMAIL_FQNS;

namespace YAMAIL_NS_ALIAS { using namespace YAMAIL_FQNS; }
namespace YAMAIL_NS_ALIAS_SHORT { using namespace YAMAIL_FQNS; }

#endif // _YAMAIL_NAMESPACE_H_

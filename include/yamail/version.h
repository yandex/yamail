#ifndef _YAMAIL_VERSION_H_
#define _YAMAIL_VERSION_H_
#include <yamail/config.h>

#define YAMAIL_VERSION 100000

#define YAMAIL_VERSION_MAJOR (YAMAIL_VERSION / 100000)
#define YAMAIL_VERSION_MINOR (YAMAIL_VERSION / 100 & 1000)
#define YAMAIL_VERSION_PATCH (YAMAIL_VERSION % 100)

// Date based version
#define YAMAIL_REVISION_DATE 20140115000

// SCCS based REVISION
#define YAMAIL_REVISION_SCCS 1000

#define YAMAIL_VERSION_STRING "1_00"

#endif // _YAMAIL_VERSION_H_

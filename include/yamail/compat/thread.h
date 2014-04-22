#ifndef _YAMAIL_COMPAT_THREAD_H_
#define _YAMAIL_COMPAT_THREAD_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

#include <yamail/compat/mutex.h>

#if defined(HAVE_STD_THREAD) && HAVE_STD_THREAD
# include <atomic>
# include <future>
# include <thread>
#else 
# include <boost/atomic.hpp>
# include <boost/thread.hpp>
#endif // HAVE_STD_THREAD

YAMAIL_FQNS_COMPAT_BEGIN

#if defined(HAVE_STD_THREAD) && HAVE_STD_THREAD
// atomic
using std::atomic;
using std::atomic_flag;
using std::memory_order;

// future
using std::promise;
using std::packaged_task;
using std::future;
using std::shared_future;
using std::future_error;
using std::future_errc;
using std::future_status;
using std::launch;
using std::async;
using std::future_category;

// thread
using std::thread;
using std::this_thread;

#else
// atomic
using boost::atomic;
using boost::atomic_flag;
using boost::memory_order;

// future
using boost::promise;
using boost::packaged_task;
using boost::future;
using boost::shared_future;
using boost::future_error;
using boost::future_errc;
using boost::future_status;
using boost::launch;
using boost::async;
using boost::future_category;

// thread
using boost::thread;
using boost::this_thread;
#endif

YAMAIL_FQNS_COMPAT_END 

#endif // _YAMAIL_COMPAT_THREAD_H_

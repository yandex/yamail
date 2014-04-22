#ifndef _YAMAIL_COMPAT_THREAD_H_
#define _YAMAIL_COMPAT_THREAD_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

#if defined(HAVE_STD_THREAD)
# include <atomic>
# include <condition_variable>
# include <future>
# include <mutex>
# include <thread>
#else 
# include <boost/atomic.hpp>
# include <boost/thread.hpp>
#endif // HAVE_STD_THREAD

YAMAIL_FQNS_COMPAT_BEGIN

#if defined(HAVE_STD_THREAD)
// atomic
using std::atomic;
using std::atomic_flag;
using std::memory_order;

// condition_variable
using std::condition_variable;
using std::condition_variable_any;
using std::cv_status;
using std::notify_all_at_thread_exit;

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

// mutex
using std::mutex;
using std::recursive_mutex;
using std::timed_mutex;
using std::recursive_timed_mutex;
using std::lock_guard;
using std::unique_lock;
using std::once_flag;
using std::adopt_lock_t;
using std::defer_lock_t;
using std::try_to_lock_t;
using std::try_lock;
using std::lock;
using std::call_once;

// thread
using std::thread;
using std::this_thread;

#else
// atomic
using boost::atomic;
using boost::atomic_flag;
using boost::memory_order;

// condition_variable
using boost::condition_variable;
using boost::condition_variable_any;
using boost::cv_status;
using boost::notify_all_at_thread_exit;

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

// mutex
using boost::mutex;
using boost::recursive_mutex;
using boost::timed_mutex;
using boost::recursive_timed_mutex;
using boost::lock_guard;
using boost::unique_lock;
using boost::once_flag;
using boost::adopt_lock_t;
using boost::defer_lock_t;
using boost::try_to_lock_t;
using boost::try_lock;
using boost::lock;
using boost::call_once;

// thread
using boost::thread;
using boost::this_thread;
#endif

YAMAIL_FQNS_COMPAT_END 

#endif // _YAMAIL_COMPAT_THREAD_H_

#ifndef _YAMAIL_COMPAT_MUTEX_H_
#define _YAMAIL_COMPAT_MUTEX_H_
#include <yamail/config.h>
#include <yamail/compat/config.h>
#include <yamail/compat/namespace.h>

#if defined(HAVE_STD_THREAD) && HAVE_STD_THREAD
# include <mutex>
# include <condition_variable>
#else 
# include <boost/thread/mutex.hpp>
# include <boost/thread/recursive_mutex.hpp>
# include <boost/thread/lock_guard.hpp>
# include <boost/thread/once.hpp>
# include <boost/thread/locks.hpp>
# include <boost/thread/reverse_lock.hpp>

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/cv_status.hpp>
#endif // HAVE_STD_THREAD

YAMAIL_FQNS_COMPAT_BEGIN

#if defined(HAVE_STD_THREAD) && HAVE_STD_THREAD
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

template <typename Lock>
class reverse_lock
{
	Lock& lock_;
public:
                  reverse_lock  (reverse_lock const&)    = delete;
    reverse_lock& operator=     (reverse_lock const&)    = delete;
  explicit inline reverse_lock  (Lock& m) : lock_ (m) { lock_.unlock (); }
           inline ~reverse_lock ()                    {   lock_.lock (); }
};

// condition_variable
using std::condition_variable;
using std::condition_variable_any;
using std::cv_status;
using std::notify_all_at_thread_exit;
#else
// mutex
using boost::mutex;
using boost::recursive_mutex;
using boost::timed_mutex;
using boost::recursive_timed_mutex;
using boost::lock_guard;
using boost::unique_lock;
using boost::reverse_lock;
using boost::once_flag;
using boost::adopt_lock_t;
using boost::defer_lock_t;
using boost::try_to_lock_t;
using boost::try_lock;
using boost::lock;
using boost::call_once;

// condition_variable
using boost::condition_variable;
using boost::condition_variable_any;
using boost::cv_status;
using boost::notify_all_at_thread_exit;
#endif
YAMAIL_FQNS_COMPAT_END 

#endif // _YAMAIL_COMPAT_MUTEX_H_

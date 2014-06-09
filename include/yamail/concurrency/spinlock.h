#ifndef _YAMAIL_CONCURRENCY_SPINLOCK_H_
#define _YAMAIL_CONCURRENCY_SPINLOCK_H_
#include <yamail/config.h>
#include <yamail/concurrency/namespace.h>

#include <yamail/compat/thread.h>

YAMAIL_FQNS_CONCURRENCY_BEGIN

class spinlock {
private:
  typedef enum {Locked, Unlocked} LockState;
  compat::atomic<LockState> state_;

public:
  spinlock() : state_(Unlocked) {}

  void lock()
  {
    while (state_.exchange(Locked, compat::memory_order_acquire) == Locked) {
      /* busy-wait */
    }
  }
  void unlock()
  {
    state_.store(Unlocked, compat::memory_order_release);
  }
};

YAMAIL_FQNS_CONCURRENCY_END

#endif // _YAMAIL_CONCURRENCY_SPINLOCK_H_

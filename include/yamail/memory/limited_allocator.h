#ifndef _YAMAIL_MEMORY_ALLOCATOR_H_
#define _YAMAIL_MEMORY_ALLOCATOR_H_
#include <yamail/config.h>
#include <yamail/memory/namespace.h>
#include <yamail/memory/limiters.h>

#include <memory>

#include <yamail/traits/enable_if_convertible.h>
#include <yamail/traits/enable_if_value_or_ref.h>

YAMAIL_NS_BEGIN
YAMAIL_NS_MEMORY_BEGIN

namespace compat = YAMAIL_FQNS_COMPAT;

/*
 * Concept of limiter of allocator
class limiter
{
    void acquire(size_t n) throw(limiter_exhausted);
    void release(size_t n) _noexcept;
};
 */

template <typename T, typename Limiter, typename BaseAllocator = std::allocator<T> >
class limited_allocator: public BaseAllocator::template rebind<T>::other
{
  typedef typename BaseAllocator::template rebind<T>::other base_t;
  typedef Limiter limiter_t;

public:
  typedef typename base_t::size_type        size_type;
  typedef typename base_t::difference_type  difference_type;
  typedef typename base_t::pointer          pointer;
  typedef typename base_t::const_pointer    const_pointer;
  typedef typename base_t::reference        reference;
  typedef typename base_t::const_reference  const_reference;
  typedef typename base_t::value_type       value_type;

public:
  template <typename U> struct rebind {
    typedef limited_allocator<U, limiter_t,
            typename base_t::template rebind<U>::other> other;
  };

  limited_allocator() _noexcept {}

  limited_allocator(limiter_t const& limiter) _noexcept
    : base_t()
    , limiter_(limiter)
  {
  }

  template <typename U, typename A>
  limited_allocator (limited_allocator<U,limiter_t,A> const& allocator,
      typename traits::enable_if_convertible<A, base_t>::type* = 0
  ) _noexcept
    : base_t (allocator)
    , limiter_(allocator.limiter())
  {
  }

  pointer allocate (size_type n,
      typename base_t::template rebind<void>::other::const_pointer hint=0)
  {
    limiter_.acquire(n * sizeof(T));
    
    pointer p;
    try {
      p = this->base_t::allocate(n, hint);
    }
    catch (...)
    {
      limiter_.release(n * sizeof(T));
      throw;
    }

    return p;
  }

  void deallocate (pointer p, size_type n)
  {
    limiter_.release(n * sizeof(T));
    this->base_t::deallocate(p, n);
  }

  size_type max_size () const _noexcept;

  limiter_t limiter() const
  { return limiter_; }

private:
  limiter_t limiter_;
};


YAMAIL_NS_MEMORY_END
YAMAIL_NS_END

#endif // _YAMAIL_MEMORY_ALLOCATOR_H_

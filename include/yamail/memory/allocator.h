#ifndef _YAMAIL_MEMORY_ALLOCATOR_H_
#define _YAMAIL_MEMORY_ALLOCATOR_H_
#include <yamail/config.h>
#include <yamail/memory/namespace.h>

#include <yamail/traits/enable_if_convertible.h>
#include <yamail/traits/enable_if_value_or_ref.h>

// #include <yamail/data/select_map.h>

#include <memory>

YAMAIL_NS_BEGIN
YAMAIL_NS_MEMORY_BEGIN

class config
{

};

template <typename T, typename Config, 
         typename BaseAllocator = std::allocator<T> >
class allocator;

template <typename BaseAllocator, typename Config>
struct allocator<void, Config, BaseAllocator> 
  : BaseAllocator::template rebind<void>::other
{
  template <typename U> struct rebind {
    typedef allocator<U, Config, 
            typename BaseAllocator::template rebind<U>::other> other;
  };
};

template <typename T, typename Config, typename BaseAllocator>
class allocator: public BaseAllocator::template rebind<T>::other
{
  typedef typename BaseAllocator::template rebind<T>::other base_t;
  typedef Config config_type;

public:
  template <typename U> struct rebind {
    typedef allocator<U, Config, 
            typename base_t::template rebind<U>::other> other;
  };

  typedef typename allocator::size_type size_type;
  typedef typename allocator::pointer pointer;
  typedef typename allocator::const_pointer const_pointer;

  allocator () _noexcept {}

  template <typename U, typename A>
  allocator (allocator<U,config_type,A>&& alloc,
      typename traits::enable_if_convertible<A, base_t>::type* = 0
  ) _noexcept 
    : base_t (std::move (alloc)) 
  {
  }

  template <typename U, typename A>
  allocator (allocator<U,config_type,A> const& alloc,
      typename traits::enable_if_convertible<A, base_t>::type* = 0
  ) _noexcept 
    : base_t (alloc) 
  {
  }

  template <typename Cfg, typename Alloc>
  allocator (Cfg&& cfg, Alloc&& alloc,
        typename traits::enable_if_value_or_ref<Cfg, config_type>::type* = 0
      , typename traits::enable_if_convertible<Alloc, base_t>::type* = 0
  )  _noexcept
    : base_t (std::forward<Alloc> (alloc))
    , config_ (std::forward<Cfg> (cfg))
  {
  }

  template <typename Cfg>
  allocator (Cfg&& cfg,
        typename traits::enable_if_value_or_ref<Cfg, config_type>::type* = 0
  )  _noexcept
    : base_t ()
    , config_ (std::forward<Cfg> (cfg))
  {
  }

  pointer allocate (size_type n, 
      typename base_t::template rebind<void>::other::const_pointer hint=0)
  {
    // ... check if memory is available ...
    config_.check (n);
    
    pointer p;

    try { 
      p = this->base_t::allocate (n, hint);
    }
    catch (...)
    {
      // ... roll back memory counters ...
      config_.free (n);
      throw;
    }

    return p;
  }

  void deallocate (pointer p, size_type n)
  {
    // ... redo memory counters ...
    config_.free (n);
    this->base_t::deallocate (p, n);
  }

  size_type max_size () const _noexcept;

private:
  config_type config_;
};


#if 0
namespace keywords {

struct hashed {};
struct ordered {};

BOOST_PARAMETER_TEMPLATE_KEYWORD(key_type)
BOOST_PARAMETER_TEMPLATE_KEYWORD(value_type)
BOOST_PARAMETER_TEMPLATE_KEYWORD(hash_type)
BOOST_PARAMETER_TEMPLATE_KEYWORD(equal_type)
BOOST_PARAMETER_TEMPLATE_KEYWORD(cmp_type)
BOOST_PARAMETER_TEMPLATE_KEYWORD(index_type)
BOOST_PARAMETER_TEMPLATE_KEYWORD(alloc_type)

} // namespace keywords
#endif

YAMAIL_NS_MEMORY_END
YAMAIL_NS_END

#endif // _YAMAIL_MEMORY_ALLOCATOR_H_

#ifndef _YAMAIL_ITERATOR_BOUNDARY_ITERATOR_H_
#define _YAMAIL_ITERATOR_BOUNDARY_ITERATOR_H_
#include <yamail/config.h>
#include <yamail/iterator/namespace.h>

#include <yamail/compat/move.h>

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_traits.hpp>

#include <boost/utility/enable_if.hpp>
#include <boost/logic/tribool.hpp>

#include <cassert>

YAMAIL_FQNS_ITERATOR_BEGIN

////////////////////////////////////////////////////////////////////////////////
// translate one iterator to another with codecvt.

namespace detail {
struct eof_t {};
}

namespace {
	const detail::eof_t eof = detail::eof_t ();
};

template <
    typename Base
  , typename Finder
>
class boundary_iterator
  : public boost::iterator_adaptor<
              boundary_iterator<Base, Finder>
            , Base
    >
{
private:
  typedef boundary_iterator self_t;
  struct enabler {};

public:
  explicit boundary_iterator (Finder finder = Finder ())
    : self_t::iterator_adaptor_ (0)
    , eof_ (boost::indeterminate)
    , finder_ (compat::move (finder))
  {
  }

  explicit boundary_iterator (Base base, Finder finder = Finder ())
    : self_t::iterator_adaptor_ (base)
    , eof_ (boost::indeterminate)
    , finder_ (compat::move (finder))
  {
  }

  explicit boundary_iterator (Base base, detail::eof_t, 
      Finder finder = Finder ())
    : self_t::iterator_adaptor_ (base)
    , eof_ (true)
    , finder_ (compat::move (finder))
  {
  }

  // XXX: ctor for other base

private:
  friend class boost::iterator_core_access;

  void increment () 
  {
  	bool was_eof = (eof_ == true);
  	assert (! was_eof);
  	eof_ = boost::indeterminate;
  	finder_.increment ();
  	++this->base_reference ();
  }

  template <class OtherBase, class OtherFinder>
  bool equal (boundary_iterator<OtherBase, OtherFinder> const& x,
      typename boost::enable_if_c<
          boost::is_convertible<OtherBase*, Base*>::value &&
            boost::is_convertible<OtherFinder*, Finder*>::value
        , enabler
      >::type = enabler ()
  ) const
  {
  	if (indeterminate (eof_) && ! bool(eof_ = this->finder_.is_eof ()))
    	  this->finder_.dereference (*this->base ());
    
std::cout << "equal: " << eof_ << " : " << x.eof_ << "\n";
    bool b = indeterminate (x.eof_);
std::cout << "b = " << b << "\n";
    if (b && (x.eof_ = x.finder_.is_eof ()) == false)
    	abort ();

  	if (indeterminate (x.eof_) && ! bool(x.eof_ = x.finder_.is_eof ()))
    {
    	std::cout << "BUG\n";
    	x.finder_.dereference (*x.base ());
    }

std::cout << "equal2: " << eof_ << " : " << x.eof_ << "\n";
    if (this->eof_ && x.eof_) return true;
    if (this->eof_ || x.eof_) return false;
  	return this->base () == x.base ();
  }

#if 0
  typename self_t::iterator_adaptor_::reference dereference() const
  {
  	return typename self_t::iterator_adaptor_::reference ();
  }
#endif

private:
  mutable boost::tribool eof_;
  Finder finder_;
};

template <typename Base, typename Finder>
boundary_iterator<Base,Finder>
make_boundary_iterator (Base base, Finder finder)
{
	return boundary_iterator<Base,Finder> (std::move (base), std::move (finder));
}

template <typename Base, typename Finder>
boundary_iterator<Base,Finder>
make_boundary_iterator (Base base, detail::eof_t e, Finder finder)
{
	return boundary_iterator<Base,Finder> (std::move(base), e, std::move(finder));
}

YAMAIL_FQNS_ITERATOR_END

#endif // _YAMAIL_ITERATOR_BOUNDARY_ITERATOR_H_

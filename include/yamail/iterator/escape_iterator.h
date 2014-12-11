#ifndef _YAMAIL_ITERATOR_ESCAPE_ITERATOR_H_
#define _YAMAIL_ITERATOR_ESCAPE_ITERATOR_H_
#include <yamail/config.h>
#include <yamail/iterator/namespace.h>

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_traits.hpp>

#include <boost/range/algorithm/find.hpp>

YAMAIL_FQNS_ITERATOR_BEGIN

////////////////////////////////////////////////////////////////////////////////
// escapes some characters

template <
    typename Base
>
class escape_iterator
  : public boost::iterator_adaptor<
              escape_iterator<Base>
            , Base
            , boost::use_default // Value
            , boost::forward_traversal_tag
    >
{
public:
  typedef typename escape_iterator::value_type value_type;

  escape_iterator ()
    : escape_ ('\0')
  {
  }

  template <typename Rng>
  escape_iterator (Base const& iter, Rng const& chars, 
      value_type escape = '\\')
    : escape_iterator::iterator_adaptor_ (iter)
    , chars_ (boost::begin (chars), boost::end (chars))
    // :, chars_ (chars)
    , escape_ (escape)
    , escape_mode_ (_no)
  {
  }

  explicit escape_iterator (Base const& iter)
    : escape_iterator::iterator_adaptor_ (iter)
    , escape_ ('\0')
    , escape_mode_ (_no)
  {
  }

private:
  friend class boost::iterator_core_access;

  typename escape_iterator::reference 
  dereference() const
  {
  	if (escape_mode_ == _1st) return escape_;


	  if (escape_mode_ == _no && 
	  	  boost::find (chars_, *this->base ()) != chars_.end ())
    {
    	escape_mode_ = _1st;
  		return escape_;
    }

    return *this->base ();
  }

  void increment () 
  {
  	switch (escape_mode_) {
  		case _1st: escape_mode_ = _2nd; return;
  		case _2nd: escape_mode_ = _no; 
  		default: ;
    }

  	++this->base_reference ();
  }

  std::basic_string<value_type>     chars_;
  const value_type                  escape_;
  mutable enum { _no, _1st, _2nd }  escape_mode_;
};

template <typename Base>
escape_iterator<Base> 
make_escape_iterator (Base base)
{
  return escape_iterator<Base> (base);
}

template <typename Base, typename Range, typename C>
escape_iterator<Base> 
make_escape_iterator (Base base, Range const& chars, C escape)
{
  return escape_iterator<Base> (base, chars, escape);
}

YAMAIL_FQNS_ITERATOR_END

#endif // _YAMAIL_ITERATOR_ESCAPE_ITERATOR_H_

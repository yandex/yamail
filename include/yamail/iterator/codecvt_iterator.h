#ifndef _YAMAIL_ITERATOR_CODECVT_ITERATOR_H_
#define _YAMAIL_ITERATOR_CODECVT_ITERATOR_H_
#include <yamail/config.h>
#include <yamail/iterator/namespace.h>

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_traits.hpp>

YAMAIL_FQNS_ITERATOR_BEGIN

////////////////////////////////////////////////////////////////////////////////
// translate one iterator to another with codecvt.

template <
    typename Base
  , typename CodeCvt
>
class codecvt_iterator
  : public boost::iterator_adaptor<
              codecvt_iterator<Base, CodeCvt>
            , Base
            , typename CodeCvt::extern_type
            , boost::single_pass_traversal_tag
            , typename CodeCvt::extern_type
    >
{
  friend class boost::iterator_core_access;

  typedef typename CodeCvt::intern_type int_char_type;
  typedef typename CodeCvt::extern_type ext_char_type;
  typedef typename CodeCvt::state_type state_type;
  typedef typename CodeCvt::result result_type;

  typedef boost::iterator_adaptor<
              codecvt_iterator<Base, CodeCvt, CharT>
            , Base
            , ext_char_type
            , boost::single_pass_traversal_tag
            , ext_char_type
  > super_t;

  typedef codecvt_iterator this_t;

  std::vector<int_char_type> int_buf_;
  std::vector<ext_char_type> ext_buf_;

  inline ExtCharT dereference () const
  {
    return const_cast<this_t*> (this)->dereference_impl ();
  }

}

YAMAIL_FQNS_ITERATOR_END

#endif // _YAMAIL_ITERATOR_CODECVT_ITERATOR_H_

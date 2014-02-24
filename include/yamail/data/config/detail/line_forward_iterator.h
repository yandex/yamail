#ifndef _YAMAIL_DATA_CONFIG_DETAIL_LINE_FORWARD_ITERATOR_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_LINE_FORWARD_ITERATOR_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <iterator>

#include <boost/iterator/iterator_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_convertible.hpp>

#include <boost/spirit/include/support_multi_pass.hpp>
#if YAMAIL_DATA_CONFIG_USE_LINE_POS_ITERATOR
#include <boost/spirit/include/support_line_pos_iterator.hpp>
#else
#include <boost/spirit//include/classic_position_iterator.hpp>
#endif

YAMAIL_FQNS_DATA_CP_BEGIN

namespace detail {

namespace spirit = boost::spirit;

using boost::enable_if;
using boost::is_convertible;

template <typename Iterator, class Enable = void>
struct convert_to_forward_iterator
{
  typedef Iterator iterator_type;
  typedef spirit::multi_pass<Iterator> type;
  
  static inline type 
  begin (iterator_type& first, iterator_type const& last)
  {
    return spirit::make_default_multi_pass (first);
  }

  static inline type 
  end (iterator_type const& first, iterator_type& last)
  {
    return spirit::make_default_multi_pass (last);
  }

};

template <typename ForwardIterator>
struct convert_to_forward_iterator<
  ForwardIterator
  , typename enable_if<
      is_convertible<
          typename boost::iterator_category<ForwardIterator>::type
        , std::forward_iterator_tag
      >
    >::type
>
{
  typedef ForwardIterator type;
  
  static inline type 
  begin (ForwardIterator first, ForwardIterator last)
  {
    return first;
  }

  static inline type 
  end (ForwardIterator first, ForwardIterator last)
  {
    return last;
  }

};

template <typename ForwardIterator>
struct convert_to_line_iterator
{
  typedef ForwardIterator forward_iterator_type;

#if YAMAIL_DATA_CONFIG_USE_LINE_POS_ITERATOR
  typedef spirit::line_pos_iterator<forward_iterator_type> type;

  static inline type begin (forward_iterator_type const& first,
      forward_iterator_type const& last)
  {
    return type (first);
  }

  static inline type end (forward_iterator_type const& first,
      forward_iterator_type const& last)
  {
    return type (first);
  }

#else
  typedef spirit::classic::position_iterator<forward_iterator_type> type;

  static inline type begin (forward_iterator_type const& first,
      forward_iterator_type const& last)
  {
    return type (first, last);
  }

  static inline type end (forward_iterator_type const&,
      forward_iterator_type const&)
  {
    return type ();
  }
#endif

};

#if YAMAIL_DATA_CONFIG_USE_LINE_POS_ITERATOR
template <typename ForwardIterator>
struct convert_to_line_iterator<spirit::line_pos_iterator<ForwardIterator > >
{
  typedef ForwardIterator forward_iterator_type;
  typedef spirit::line_pos_iterator<forward_iterator_type> type;

  static inline type begin (type first, type last) { return first; }
  static inline type end (type first, type last) { return last; }
};
#else
template <typename ForwardIterator, typename P, typename S>
struct convert_to_line_iterator<
  spirit::classic::position_iterator<ForwardIterator,P,S > >
{
  typedef ForwardIterator forward_iterator_type;
  typedef spirit::classic::position_iterator<forward_iterator_type,P,S> type;

  static inline type begin (type first, type last) { return first; }
  static inline type end (type first, type last) { return last; }
};
#endif

// do both...
template <typename Iterator>
struct wrap_line_forward_iterator
{
  typedef typename convert_to_forward_iterator<Iterator>::type 
    forward_iterator;

  typedef typename convert_to_line_iterator<forward_iterator>::type
    type;

  static inline type begin (Iterator& first, Iterator& last)
  {
    return convert_to_line_iterator<forward_iterator>::begin (
              convert_to_forward_iterator<Iterator>::begin (first, last),
              convert_to_forward_iterator<Iterator>::end (first, last)
    );
  }

  static inline type end (Iterator& first, Iterator& last)
  {
    return convert_to_line_iterator<forward_iterator>::end (
              convert_to_forward_iterator<Iterator>::begin (first, last),
              convert_to_forward_iterator<Iterator>::end (first, last)
    );
  }

};

template <typename Iterator>
typename wrap_line_forward_iterator<Iterator>::type
line_forward_iterator_begin (Iterator& first, Iterator& last)
{
  return wrap_line_forward_iterator<Iterator>::begin (first, last);
}

template <typename Iterator>
typename wrap_line_forward_iterator<Iterator>::type
line_forward_iterator_end (Iterator& first, Iterator& last)
{
  return wrap_line_forward_iterator<Iterator>::end (first, last);
}


} // namespace detail

YAMAIL_FQNS_DATA_CP_END

#endif // _YAMAIL_DATA_CONFIG_DETAIL_LINE_FORWARD_ITERATOR_H_

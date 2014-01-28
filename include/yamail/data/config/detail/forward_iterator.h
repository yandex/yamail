#ifndef _YAMAIL_DATA_CONFIG_DETAIL_FORWARD_ITERATOR_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_FORWARD_ITERATOR_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <iterator>

#include <boost/iterator/iterator_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_convertible.hpp>

#include <boost/spirit/include/support_multi_pass.hpp>

YAMAIL_FQNS_DATA_CP_BEGIN

namespace detail {

namespace spirit = boost::spirit;

using boost::enable_if;
using boost::is_convertible;

template <typename Iterator, class Enable = void>
struct convert_to_forward
{
  typedef Iterator iterator_type;
  typedef spirit::multi_pass<Iterator> type;
  
  static inline
  type 
  create (iterator_type& iter)
  {
    return spirit::make_default_multi_pass (iter);
  }
};

template <typename ForwardIterator>
struct convert_to_forward<
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
  create (ForwardIterator i) { return i; }
};

} // namespace detail

YAMAIL_FQNS_DATA_CP_END

#endif // _YAMAIL_DATA_CONFIG_DETAIL_FORWARD_ITERATOR_H_

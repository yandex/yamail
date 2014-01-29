#ifndef _YAMAIL_DATA_CONFIG_PARSE_ITERATOR_H_
#define _YAMAIL_DATA_CONFIG_PARSE_ITERATOR_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/data/config/detail/forward_iterator.h>
#include <yamail/data/config/detail/ast_cache.h>
#include <yamail/data/config/detail/parse.h>

#include <boost/spirit/include/support_line_pos_iterator.hpp>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

YAMAIL_FQNS_DATA_CP_BEGIN

namespace spirit = boost::spirit;

template <typename Iterator>
inline bool
parse (ast& nodes, Iterator first, Iterator last, 
    paths_type const& include_dirs = paths_type ())
{
  default_custom_error_handler eh;
  return parse (nodes, first, last, eh, include_dirs);
}

template <typename Iterator, typename UserErrorHandler>
inline
bool
parse (ast& nodes, Iterator first, Iterator last, 
    UserErrorHandler& custom_handler,
    paths_type const& include_dirs = paths_type ())
{
  typedef detail::convert_to_forward<Iterator> forward_cvt;
  typedef typename forward_cvt::type forward_iterator_type;

  forward_iterator_type f_first (forward_cvt::create (first));
  forward_iterator_type f_last (forward_cvt::create (last));

  typedef spirit::line_pos_iterator<forward_iterator_type> line_iterator_type;
  line_iterator_type l_first (f_first);
  line_iterator_type l_last (f_last);

  typedef error_handler<UserErrorHandler> error_handler_type;
  typedef detail::include_handler<error_handler_type> include_handler_type;
  typedef detail::ast_cache<error_handler_type, include_handler_type> 
    cache_type;

#if 0
  BOOST_MPL_ASSERT ((
    boost::is_convertible<
      typename boost::iterator_category<Iterator>::type,
      std::forward_iterator_tag
    >
  ));
#endif
  BOOST_MPL_ASSERT ((
    boost::is_convertible<
      typename boost::iterator_category<forward_iterator_type>::type,
      std::forward_iterator_tag
    >
  ));

#if 0
  BOOST_MPL_ASSERT ((
    boost::is_convertible<
      typename boost::iterator_category<line_iterator_type>::type,
      std::forward_iterator_tag
    >
  ));
#endif

  error_handler_type    ehandler (custom_handler);
  cache_type            cache;

  include_handler_type  ihandler = 
    include_handler_type (ehandler, include_dirs, cache);


  return detail::parse (nodes, l_first, l_last, include_dirs, cache, 
      ehandler, ihandler);
}

#if 0
template <typename Iterator>
inline
bool
parse (ast& nodes, boost::iterator_range<Iterator> range,
    paths_type const& include_dirs = paths_type ())
{
  return parse (nodes, boost::begin (range), boost::end (range),
      include_dirs);
}
#endif

YAMAIL_FQNS_DATA_CP_END

#endif // _YAMAIL_DATA_CONFIG_PARSE_ITERATOR_H_

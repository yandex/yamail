#ifndef _YAMAIL_DATA_CONFIG_DETAIL_PARSE_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_PARSE_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <iterator>

#include <boost/mpl/assert.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/type_traits/is_convertible.hpp>

#include <yamail/data/config/ast.h>
#include <yamail/data/config/paths_type.h>
#include <yamail/data/config/grammar.h>
#include <yamail/data/config/detail/line_forward_iterator.h>

YAMAIL_FQNS_DATA_CP_BEGIN
namespace detail {

namespace spirit = boost::spirit;

template <
    typename Iterator
  , typename Opener
  , typename ErrorHandler
  , typename IncludeHandler
>
bool
parse (ast& nodes, std::string const& path, 
    Iterator first, Iterator last, 
    paths_type const& include_dirs, 
    Opener& opener,
    ErrorHandler ehandler, IncludeHandler ihandler)
{
  auto i_first = line_forward_iterator_begin (first, last);


  spirit::classic::file_position pos = { path, 1, 1 };
  i_first.set_position (pos);

  return parse_wrapped (
      nodes, 
      i_first, line_forward_iterator_end (first, last),
      include_dirs, opener, ehandler, ihandler);
}

template <
    typename Iterator
  , typename Opener
  , typename ErrorHandler
  , typename IncludeHandler
>
bool
parse (ast& nodes, Iterator first, Iterator last, 
    paths_type const& include_dirs, 
    Opener& opener,
    ErrorHandler ehandler, IncludeHandler ihandler)
{
  return parse_wrapped (
      nodes, 
      line_forward_iterator_begin (first, last), 
      line_forward_iterator_end (first, last),
      include_dirs, opener, ehandler, ihandler);
}

template <
    typename ForwardIterator
  , typename Opener
  , typename ErrorHandler
  , typename IncludeHandler
>
bool
parse_wrapped (ast& nodes, ForwardIterator first, ForwardIterator last, 
    paths_type const& include_dirs, 
    Opener& opener,
    ErrorHandler ehandler, IncludeHandler ihandler)
{
  BOOST_MPL_ASSERT ((
    boost::is_convertible<
      typename boost::iterator_category<ForwardIterator>::type,
      std::forward_iterator_tag
    >
  ));

  typedef grammar<ForwardIterator, ErrorHandler, IncludeHandler> parser_type;
  typedef whitespace<ForwardIterator> space_type;

  parser_type parser (ehandler, ihandler);
  space_type ws;
  bool result;

  try {
    result = phrase_parse (first, last, parser, ws, nodes);
  }
  catch (expected_component const& ex)
  {
    // XXX
    throw;
  }

  return result;
}

} // namespace detail
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_DETAIL_PARSE_H_

#ifndef _YAMAIL_DATA_CONFIG_PARSE_ITERATOR_H_
#define _YAMAIL_DATA_CONFIG_PARSE_ITERATOR_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/data/config/detail/ast_cache.h>
#include <yamail/data/config/detail/parse.h>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

YAMAIL_FQNS_DATA_CP_BEGIN

namespace spirit = boost::spirit;

template <typename Iterator>
inline bool
parse (ast& nodes, std::string const& path, Iterator first, Iterator last, 
    paths_type const& include_dirs = paths_type ())
{
  default_error_handler eh;
  return parse (nodes, path, first, last, eh, include_dirs);
}

template <typename Iterator>
inline bool
parse (ast& nodes, Iterator first, Iterator last, 
    paths_type const& include_dirs = paths_type ())
{
  return parse (nodes, "", first, last, include_dirs);
}

template <typename Iterator, typename ErrorHandler>
inline
bool
parse (ast& nodes, Iterator first, Iterator last, 
    ErrorHandler& error_handler,
    paths_type const& include_dirs = paths_type ())
{
  return parse (nodes, "", first, last, error_handler, include_dirs);
}

template <typename Iterator, typename ErrorHandler>
inline
bool
parse (ast& nodes, std::string const& path, Iterator first, Iterator last, 
    ErrorHandler& error_handler,
    paths_type const& include_dirs = paths_type ())
{
  typedef error_wrapper<ErrorHandler> error_wrapper_type;
  typedef detail::include_handler<error_wrapper_type> include_handler_type;
  typedef detail::ast_cache<error_wrapper_type, include_handler_type> 
    cache_type;

  error_wrapper_type    ehandler (error_handler);
  cache_type            cache;

  include_handler_type  ihandler = 
    include_handler_type (ehandler, include_dirs, cache);

  return detail::parse (nodes, "", first, last, 
      include_dirs, cache, ehandler, ihandler);
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

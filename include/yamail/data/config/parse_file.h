#ifndef _YAMAIL_DATA_CONFIG_PARSE_FILE_H_
#define _YAMAIL_DATA_CONFIG_PARSE_FILE_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/data/config/path_traits.h>
#include <yamail/data/config/parse_iterator.h>
#include <yamail/data/config/paths_type.h>

#include <yamail/utility/in.h>

#include <type_traits>

YAMAIL_FQNS_DATA_CP_BEGIN

template <typename Source, typename ErrorHandler, 
 typename = typename std::enable_if<path_traits::is_pathable<
    typename std::decay<Source>::type>::value>::type>
inline bool
parse (ast& nodes, Source const& path, ErrorHandler& eh, 
    paths_type const& include_dirs = paths_type ())
{
  typedef typename std::decay<ErrorHandler>::type error_handler_type;
  parse_with_error_handler<Source, error_handler_type> (
      nodes, path, std::forward<ErrorHandler> (eh), 
      include_dirs, cvt)
}

template <typename Source, typename ErrorHandler, 
 typename = typename std::enable_if<path_traits::is_pathable<
    typename std::decay<Source>::type>::value>::type>
inline bool
parse_with_error_handler (ast& nodes, Source const& path, 
    YAMAIL_NS_UTILITY::in<ErrorHandler> eh, 
    paths_type const& include_dirs = paths_type ())
{
}

template <typename ErrorHandler>
inline bool
parse_file (ast& nodes, std::string const& path, ErrorHandler& error_handler,
    paths_type const& include_dirs = paths_type ())
{
  // Use ast cache instead. Or at least use single file opener
  boost::iostreams::mapped_file_source input (path);
  return parse (nodes, path, input.data (), input.data () + input.size (), 
      error_handler, include_dirs);
}

inline bool
parse_file (ast& nodes, std::string const& path, 
    paths_type const& include_dirs = paths_type ())
{
  default_error_handler eh;
  return parse_file (nodes, path, eh, include_dirs);
}

YAMAIL_FQNS_DATA_CP_END

#endif // _YAMAIL_DATA_CONFIG_PARSE_FILE_H_

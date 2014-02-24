#ifndef _YAMAIL_DATA_CONFIG_PARSE_FILE_H_
#define _YAMAIL_DATA_CONFIG_PARSE_FILE_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/data/config/path_traits.h>
#include <yamail/data/config/parse_iterator.h>
#include <yamail/data/config/paths_type.h>
#include <yamail/data/config/error_wrapper.h>

#include <yamail/data/config/detail/parse_file.h>
#include <yamail/data/config/detail/boost_fs_handler.h>
#include <yamail/data/config/detail/ast_cache.h>

#include <yamail/utility/in.h>

#include <type_traits>

YAMAIL_FQNS_DATA_CP_BEGIN

template <typename ErrorHandler, typename FsHandler>
inline bool parse_with_error_handler (
    ast& nodes, typename FsHandler::file_path const& path, 
    paths_type const& include_dirs = paths_type (),
    YAMAIL_NS_UTILITY::in<ErrorHandler> eh, 
    YAMAIL_NS_UTILITY::rw_in<FsHandler> fs)
{
  std::cout << __PRETTY_FUNCTION__ << "\n";

  detail::ast_cache<error_wrapper<ErrorHandler>, FsHandler> cacher;
  detail::boost_fs_handler fs;

  return detail::parse (nodes, path, include_dirs, 
      eh.get (), fs.get (), cacher);
}

template <
    typename ErrorHandler = default_error_handler
  , typename FsHandler = boost_fs_handler
>
inline bool
parse (ast& nodes, typename FsHandler::file_path const& path, 
    ErrorHandler&& eh = ErrorHandler (),
    paths_type const& include_dirs = paths_type (),
    FsHandler&& fs = FsHandler ())
{
  typedef typename std::decay<ErrorHandler>::type error_handler_type;
  typedef typename std::decay<FsHandler>::type fs_handler_type;

  return parse_with_error_handler<error_handler_type, fs_handler_type> (
      nodes, path, include_dirs, 
      std::forward<ErrorHandler> (eh), std::forward<FsHandler> (fs));
}

template <
    typename ErrorHandler = default_error_handler
  , typename FsHandler = boost_fs_handler
>
inline bool
parse (ast& nodes, typename FsHandler::file_path const& path, 
    paths_type const& include_dirs, ErrorHandler&& eh = ErrorHandler (),
    FsHandler&& fs = FsHandler ())
{
  typedef typename std::decay<ErrorHandler>::type error_handler_type;
  typedef typename std::decay<FsHandler>::type fs_handler_type;

  return parse_with_error_handler<error_handler_type, fs_handler_type> (
      nodes, path, include_dirs, 
      std::forward<ErrorHandler> (eh), std::forward<FsHandler> (fs));
}

YAMAIL_FQNS_DATA_CP_END

#endif // _YAMAIL_DATA_CONFIG_PARSE_FILE_H_

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

template <
    typename FilePath, typename IncludeDirs
  , typename ErrorHandler, typename Fs>
struct parse_helper
{
  parse_helper (rw_in<IncludeDirs> const& dirs, ErrorHandler& eh)
    : dirs_ (dirs)
    , eh_ (eh)
  {
  }

  template <typename Ast, typename Path, 
           typename Iter1, typename Iter2, typename Opener>
  bool operator() (Ast&& ast, Path&& path, 
      Iter1&& first, Iter2&& last, Opener&& operner) const
  {
    return detail::parse_iterator (
      nodes, path_, dirs, first, last,
      eh_, opener);
  }

  IncludeDirs& dirs;
  ErrorHandler& eh_;
};



template <typename ErrorHandler, typename FsHandler>
inline bool parse_with_error_handler (
    ast& nodes, typename FsHandler::file_path const& path, 
    paths_type const& include_dirs = paths_type (),
    YAMAIL_NS_UTILITY::in<ErrorHandler> eh, 
    YAMAIL_NS_UTILITY::rw_in<FsHandler> fs)
{
  std::cout << __PRETTY_FUNCTION__ << "\n";

  return detail::parse (nodes, path, dirs, make_parse_helper (dirs, eh));
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

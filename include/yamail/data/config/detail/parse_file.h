#ifndef _YAMAIL_DATA_CONFIG_DETAIL_PARSE_FILE_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_PARSE_FILE_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/data/config/ast.h>
#include <yamail/data/config/path_traits.h>
#include <yamail/data/config/paths_type.h>

#include <yamail/data/config/detail/boost_fs_handler.h>
#include <yamail/data/config/detail/ast_cache.h>

// #include <yamail/data/config/detail/parse.h>

// #include <boost/iostreams/device/mapped_file.hpp>
// #include <boost/spirit/include/support_line_pos_iterator.hpp>

#include <string>
#include <iostream> // debug only

YAMAIL_FQNS_DATA_CP_BEGIN
namespace detail {


template <
    typename ErrorWrapper
  , typename FsHandler
  , typename IncludeDirs
  , typename IncludeOpener
> 
inline bool 
parse (
    ast& nodes, 
    typename FsHandler::file_path const& path,
    IncludeDirs const& include_dirs,
    ErrorWrapper& error_handler,
    FsHandler& fs,
    IncludeOpener& include_opener)
{
  for (auto dir: include_dirs)
  {
    try { 
      auto nodes_ptr = include_opener (path, dir);
      nodes = *nodes_ptr;
      return true;
    }
    catch (typename FsHandler::error_type const& err) {
      // go to the next iteration on open errors.
    }
  }

  // should we try current dir? probably yes!

  try {
    auto nodes_ptr = include_opener (path);
    nodes = *nodes_ptr;

    // how to signal the specific error?
    return true;
  }
  catch (typename FsHandler::error_type const& err) {
    // XXX 
    // do not throw "file not found" here, it should not propogate up!!!
  }

  // XXX: signal the specific error?
  return false;
}

#if 0
template <
    typename ErrorHandler 
  , typename IncludeHandler
>
inline bool
parse_file (ast& nodes, 
    std::string const& file, paths_type const& include_dirs, 
    ast_cache<ErrorHandler, IncludeHandler>& cache,
    ErrorHandler ehandler, IncludeHandler ihandler)
{
  try {
    boost::iostreams::mapped_file_source input (file);

    typedef spirit::line_pos_iterator<char const*> line_iterator_type;
    typedef spirit::line_pos_iterator<char const*> line_iterator_type;
    line_iterator_type l_first (input.data ());
    line_iterator_type l_last (input.data () + input.size ());

    return parse (nodes, l_first, l_last, include_dirs, 
        cache, ehandler, ihandler);
  }
  catch (...) {
    std::cout << "cannot open file: " << file << std::endl;
    throw;
  }
}
#endif
} // namespace detail
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_DETAIL_PARSE_FILE_H_

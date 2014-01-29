#ifndef _YAMAIL_DATA_CONFIG_DETAIL_PARSE_FILE_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_PARSE_FILE_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/data/config/detail/ast_xache.h>
#include <yamail/data/config/detail/parse.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/spirit/include/support_line_pos_iterator.hpp>

#include <string>
#include <iostream> // debug only

YAMAIL_FQNS_DATA_CP_BEGIN
namespace detail {

template <
    typename ErrorHandler 
  , typename IncludeHandler
>
inline bool
parse_file (ast& nodes, std::string const& file, paths_type const& include_dirs, 
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
} // namespace detail
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_DETAIL_PARSE_FILE_H_

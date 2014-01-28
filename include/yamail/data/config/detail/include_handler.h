#ifndef _YAMAIL_DATA_CONFIG_DETAIL_INCLUDE_HANDLER_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_INCLUDE_HANDLER_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/data/config/ast.h>
#include <yamail/data/config/paths_type.h>
#include <yamail/data/config/detail/file_cache.h>

YAMAIL_FQNS_DATA_CP_BEGIN
namespace detail {

template <typename ErrorHandler>
struct include_handler
{
  template <typename, typename=void>
  struct result { typedef void type; };

  ErrorHandler error;

  typedef file_cache<ErrorHandler, include_handler> file_cache_type;
  file_cache_type& fcache;

  paths_type include_dirs;


  include_handler (ErrorHandler error, paths_type const& include_dirs,
          file_cache_type& fcache)
    : error (error)
    , fcache (fcache)
    , include_dirs (include_dirs)
  {
  }

  void operator() (ast_node& node, std::string const& file) const
  {
    node.value = fcache.get (file, include_dirs, error, *this);
  }
};


} // namespace detail
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_DETAIL_INCLUDE_HANDLER_H_

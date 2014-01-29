#ifndef _YAMAIL_DATA_CONFIG_DETAIL_AST_CACHE_FWD_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_AST_CACHE_FWD_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/data/config/detail/boost_fs_handler.h>

// #include <ya/data/config/error_handler.h>
// #include <ya/data/config/detail/include_handler.h>

YAMAIL_FQNS_DATA_CP_BEGIN
namespace detail {

template <
    typename ErrorHandler   // = error_handler
  , typename IncludeHandler // = include_handler<ErrorHandler>
  , typename FS = boost_fs_handler<char> 
>
class ast_cache;

} // namespace detail
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_DETAIL_AST_CACHE_FWD_H_

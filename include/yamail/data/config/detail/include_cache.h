#ifndef _YAMAIL_DATA_CONFIG_DETAIL_INCLUDE_CACHE_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_INCLUDE_CACHE_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>
#include <yamail/compat/unordered.h>
#include <yamail/data/config/detail/include_opener.h>

#include <memory>

YAMAIL_FQNS_DATA_CP_BEGIN

namespace detail {

template <
    typename Parser
  , typename FsHandler = boost_fs_handler<>
  , typename AllocT = std::allocator<ast>
>
class include_cache : public include_opener<Parser, FsHandler, AllocT>
{
public:
  typedef typename include_cache::ast_ptr   ast_ptr;
  typedef typename include_cache::path_type path_type;

protected:
  typedef include_opener<Parser, FsHandler, AllocT> base_t;
  typedef compat::unordered_map<path_type, ast_ptr> map_type;

public:
  using base_t::base_t;
  
  ast_ptr operator() (path_type const& path)
  {
    path_type normalized = this->normalize (path);

    typename map_type::iterator found = map_.find (normalized);
    if (found != map_.end ()) 
    {
      return found->second;
    }

    return map_[normalized] = this->get_normalized (normalized);
  }

private:
  map_type      map_;
};

} // namespace detail
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_DETAIL_INCLUDE_CACHE_H_

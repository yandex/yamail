#ifndef _YAMAIL_DATA_CONFIG_DETAIL_INCLUDE_DUMMY_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_INCLUDE_DUMMY_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/compat/shared_ptr.h>

#include <yamail/data/config/ast.h>
#include <yamail/data/config/paths_type.h>
#include <yamail/data/config/detail/boost_fs_handler.h>

#include <yamail/utility/in.h>

#include <list>
#include <sstream>
#include <memory>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <vector>

YAMAIL_FQNS_DATA_CP_BEGIN
namespace detail {

using utility::rw_in;

// Do nothing, fake include opener.
template <
  typename AllocT = std::allocator<ast> 
>
class include_dummy
{
public:
  typedef AllocT allocator_type;
  typedef compat::shared_ptr<ast> ast_ptr;

  include_dummy (allocator_type const& alloc = allocator_type ())
    : alloc_ (alloc)
  {
  }

  inline allocator_type& get_allocator ()
  {
    return alloc_;
  }

  ast_ptr operator() (path_type const& path)
  {
    return compat::allocate_shared<ast> (alloc_);
  }

private:
  allocator_type      alloc_;
};

template <
  : typename AllocT = std::allocator<ast> 
> inline include_dummy<AllocT> 
make_include_dummy (AllocT&& alloc = AllocT ())
{
  return include_dummy<AllocT> (std::forward<AllocT> (alloc));
}

} // namespace detail
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_DETAIL_INCLUDE_DUMMY_H_

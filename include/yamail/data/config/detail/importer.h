#ifndef _YA_DATA_CONFIG_DETAIL_IMPORTER_H_
#define _YA_DATA_CONFIG_DETAIL_IMPORTER_H_

#include <ya/data/config/ast.h>

namespace ya {
namespace data {
namespace config {

namespace detail {

template <>
bool
check_import (ast_node& a)
{
  bool processed_any = false;
  if (a.type == Import && a.value.get<ast&> () != 0) {
    // find named entry
    ast_node* found = find_node (a.name);

    if (found) {
      a.value = found;
      processed_any = true;
    }
  }

  return processed_any;
}

template <>
bool importer (ast& nodes) 
{
  BOOST_FOREACH (ast_node& node, nodes) 
  {
    check_import (node);
  }
}

} // namespace detail
}}} // namespace ya::data::config
#endif // _YA_DATA_CONFIG_DETAIL_IMPORTER_H_

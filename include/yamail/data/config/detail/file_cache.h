#ifndef _YAMAIL_DATA_CONFIG_DETAIL_FILE_CACHE_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_FILE_CACHE_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/data/config/error_handler.h>
#include <yamail/data/config/detail/file_cache_fwd.h>
#include <yamail/data/config/detail/normalize_path.h>
#include <yamail/data/config/detail/parse_file.h>
#include <yamail/data/config/detail/include_handler.h>

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <string>

YAMAIL_FQNS_DATA_CP_BEGIN

namespace detail {

namespace fs = boost::filesystem;

template <typename ErrorHandler, typename IncludeHandler>
class file_cache 
{
  typedef boost::shared_ptr<ast> ast_ptr;
  typedef boost::unordered_map<fs::path, ast_ptr> map_type;

public:

  template <typename Source>
  ast_ptr
  get (std::string const& file, 
      paths_type const& include_dirs = paths_type (), 
      ErrorHandler ehandler = ErrorHandler (),
      IncludeHandler ihandler = IncludeHandler ())
  {
    fs::path normalized = normalize_path (file);
    map_type::iterator found = map_.find (normalized);
    if (found != map_.end ()) return found->second;

    ast_ptr nodes = boost::make_shared<ast> ();
    detail::parse_file (*nodes, normalized.native (), include_dirs, *this,
        ehandler, ihandler);
    return map_[normalized] = nodes;
  }

private:
  map_type map_;
};

} // namespace detail
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_DETAIL_FILE_CACHE_H_

#ifndef _YAMAIL_DATA_CONFIG_DETAIL_AST_CACHE_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_AST_CACHE_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/compat/shared_ptr.h>
#include <yamail/compat/unordered.h>

#include <yamail/data/config/error_handler.h>
#include <yamail/data/config/detail/ast_cache_fwd.h>
#include <yamail/data/config/detail/normalize_path.h>
#include <yamail/data/config/detail/parse.h>
#include <yamail/data/config/detail/include_handler.h>
#include <yamail/data/config/detail/boost_fs_handler.h>

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <string>

YAMAIL_FQNS_DATA_CP_BEGIN

namespace detail {

template <typename ErrorHandler, typename IncludeHandler, typename FS>
class ast_cache 
{
  typedef FS fs_type;

  typedef compat::shared_ptr<ast> ast_ptr;

  // internal file name representation: wchar_t or char
  typedef char char_type;
  typedef std::basic_string<char_type> string_type;

  typedef compat::unordered_map<string_type, ast_ptr> map_type;

  class fs_guard
  {
    fs_type& fs_;
  public:
    fs_guard (fs_type& fs) : fs_ (fs) {}
    ~fs_guard () { fs_.close (); }
  };

public:
  ast_cache ()
  {
  }

  template <typename Source>
  ast_ptr
  get (Source const& path, 
      paths_type const& include_dirs = paths_type (), 
      ErrorHandler ehandler = ErrorHandler (),
      IncludeHandler ihandler = IncludeHandler ())
  {
    string_type normalized = 
      fs_type::template normalize_path_raw<string_type> (path);

    typename map_type::iterator found = map_.find (normalized);
    if (found != map_.end ()) return found->second;

    ast_ptr nodes = boost::make_shared<ast> ();

    fs_type f = fs_type (normalized);

    {
      fs_guard fg (f);
      detail::parse (*nodes, f.begin (), f.end (), include_dirs, *this,
          ehandler, ihandler);
      // autoclose f
    }

    map_[normalized] = nodes;


    return nodes;
  }

private:
  map_type map_;
};

} // namespace detail
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_DETAIL_AST_CACHE_H_

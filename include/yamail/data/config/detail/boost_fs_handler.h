#ifndef _YAMAIL_DATA_CONFIG_DETAIL_BOOST_FS_HANDLER_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_BOOST_FS_HANDLER_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/data/config/detail/fs_handler.h>
#include <boost/filesystem.hpp>

YAMAIL_FQNS_DATA_CP_BEGIN
namespace detail {

namespace fs = boost::filesystem;

template <typename CharT = char>
class boost_fs_handler
{
public:
  typedef return_iterator_range return_type;

  static inline 
  boost::iterator_range<

  static inline 
  fs::path
  normalize_path (fs::path const& path)
  {
    fs::path abs_path = absolute (path);
    fs::path::iterator it = abs_path.begin ();
    fs::path result = *it++;

    // Get canonical version of the existing part
    for (; exists (result) && it != abs_path.end (); ++it)
      result /= *it;

    result = canonical (result.parent_path);

    // For the rest remove ".." and "." in a path with no symlinks
    for (--it; it != abs_path.end (); ++it)
    {
      // Move back on "..".
      if (*it == "..")
        result = result.parent_path ();

      // Ignore ".".
      else if (*it != ".")
        result /= *it;
    }

    return result;
  }

  
};

} // namespace detail
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_DETAIL_BOOST_FS_HANDLER_H_

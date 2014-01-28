#ifndef _YAMAIL_DATA_CONFIG_DETAIL_NORMALIZE_PATH_H_
#define _YAMAIL_DATA_CONFIG_DETAIL_NORMALIZE_PATH_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <boost/filesystem.hpp>

YAMAIL_FQNS_DATA_CP_BEGIN
namespace detail {

namespace fs = boost::filesystem;

// returns normalized absolute path.

inline 
fs::path
normalize_path (fs::path const& path)
{
  fs::path absPath = absolute (path);
  fs::path::iterator it = absPath.begin ();
  fs::path result = *it++;

  // Get canonical version of the existing part
  for (; exists (result) && it != absPath.end (); ++it)
    result /= *it;

  result = canonical (result.parent_path ());

  // For the rest remove ".." and "." in a path with no symlinks
  for (--it; it != absPath.end (); ++it)
  {
    // Just move back on ".."
    if (*it == "..") 
      result = result.parent_path ();

    // Ignore "."
    else if (*it != ".")
      result /= *it;
  }

  return result;
}

} // namespace detail
YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_DETAIL_NORMALIZE_PATH_H_

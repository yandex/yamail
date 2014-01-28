#ifndef _YAMAIL_DATA_CONFIG_PARSE_FILE_H_
#define _YAMAIL_DATA_CONFIG_PARSE_FILE_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/data/config/parse_iterator.h>
#include <boost/iostreams/device/mapped_file.hpp>

YAMAIL_FQNS_DATA_CP_BEGIN

inline bool
parse (ast& nodes, std::string& path, 
    paths_type const& include_dirs = paths_type ())
{
  default_custom_error_handler eh;
  return parse (nodes, path, eh, include_dirs);
}

template <typename UserErrorHandler>
inline bool
parse (ast& nodes, std::string& path, UserErrorHandler& custom_handler,
    paths_type const& include_dirs = paths_type ())
{
  boost::iostreams::mapped_file_source input (path);
  return parse (nodes, input.data (), input.data () + input.size (), 
      custom_handler, include_dirs);
}

YAMAIL_FQNS_DATA_CP_END

#endif // _YAMAIL_DATA_CONFIG_PARSE_FILE_H_

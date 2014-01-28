#ifndef _YAMAIL_DATA_CONFIG_PARSE_STREAM_H_
#define _YAMAIL_DATA_CONFIG_PARSE_STREAM_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/data/config/parse_iterator.h>

YAMAIL_FQNS_DATA_CP_BEGIN

template <typename Stream>
inline bool
parse (ast& nodes, Stream& stream, 
    paths_type const& include_dirs = paths_type ())
{
  default_custom_error_handler eh;
  return parse (nodes, stream, eh, include_dirs);
}

template <typename Stream, typename UserErrorHandler>
inline
bool
parse (ast& nodes, Stream& stream, UserErrorHandler& custom_handler,
    paths_type const& include_dirs = paths_type ())
{
  typedef std::istreambuf_iterator<char> base_iterator_type;

  base_iterator_type first = base_iterator_type (stream);
  base_iterator_type last;

  return parse (nodes, first, last, custom_handler, include_dirs);
}

YAMAIL_FQNS_DATA_CP_END

#endif // _YAMAIL_DATA_CONFIG_PARSE_STREAM_H_

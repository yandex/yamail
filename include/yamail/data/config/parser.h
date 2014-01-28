#ifndef _YAMAIL_DATA_CONFIG_PARSER_H_
#define _YAMAIL_DATA_CONFIG_PARSER_H_

#include <vector>
#include <string>

#include <iterator>

#include <boost/iterator/traits.hpp>

namespace ya {
namespace data {
namespace config {

typedef std::vector<std::string> paths_type;

template <typename Iterator>

parse (Iterator begin, Iterator end, include_dirs lib_paths = paths_type ());

template <typename Stream>
parse (Stream& stream, paths_type include_dirs = paths_type ());

parse (std::string& path, paths_type include_dirs = paths_type ());


}}} // namespace ya::data::config
#endif // _YAMAIL_DATA_CONFIG_PARSER_H_

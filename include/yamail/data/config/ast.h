#ifndef _YAMAIL_DATA_CONFIG_AST_H_
#define _YAMAIL_DATA_CONFIG_AST_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <string>
#include <vector>

YAMAIL_FQNS_DATA_CP_BEGIN

struct ast_node;
typedef std::vector<ast_node> ast;

typedef std::vector<std::string> value_array_t;
// typedef std::vector<boost::recursive_wrapper<ast_node> > value_compound_t;
typedef boost::recursive_wrapper<std::vector<ast_node> > value_compound_t;
typedef std::string value_single_t;

struct value_void_t {};

#if 1
typedef boost::variant<
    value_void_t
  , value_compound_t
  , value_array_t
  , value_single_t
  , boost::shared_ptr<ast>  // included data
  , ast*                    // imported data
  , void*                   // binary data
> ast_value;
#else
typedef boost::make_recursive_variant<
    std::vector< boost::recursive_variant_ >
  , value_array_t
  , value_single_t
>::type ast_value;
#endif


enum etype_t {
  Other,
  Array,
  Compound,
  Module,     //< ???

  Import,     //< \ should be just import with filename(opt) and varname(opt).
  Include,    //< / 

  Binary,     //< Binary literal.
  BinFile     //< Binary data from file.
};

struct ast_node
{
  etype_t     id;
  std::string key;
  boost::optional<std::string> name;

  ast_value   value;

  std::size_t line;
  std::size_t pos;

  ast_node ()
    : id (Other)
    , line (-1)
    , pos (-1)
  {}

};

enum field_index {
  AST_ID,
  AST_KEY,
  AST_NAME,
  AST_VALUE,
  AST_LINE,
  AST_POS
};

YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_AST_H_

#ifndef _YAMAIL_DATA_CONFIG_AST_IO_H_
#define _YAMAIL_DATA_CONFIG_AST_IO_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include "ast.h"

#include <ostream>
#include <boost/foreach.hpp>

YAMAIL_NS_BEGIN
YAMAIL_NS_DATA_BEGIN
YAMAIL_NS_DATA_CP_BEGIN

namespace {
  int const tabsize = 4;
}

inline 
std::ostream& tab (std::ostream& os, int indent)
{
  while (indent-- > 0)
    os << ' ';
  return os;
}

struct ast_node_printer {
  ast_node_printer (std::ostream& os, int indent = 0)
    : os_ (os)
    , indent_ (indent)
  {
  }

  std::ostream& operator() (ast_node const& a) const;
  
  std::ostream& os_;
  int indent_;
};
    
struct ast_value_printer: boost::static_visitor<std::ostream&>
{
  ast_value_printer (std::ostream& os, int indent = 0)
    : os_ (os)
    , indent_ (indent)
  {
  }

  std::ostream& operator() (value_compound_t const& nodes) const
  {
    os_ << '{' << std::endl;

    BOOST_FOREACH (ast_node const& node, nodes.get ())
    {
      ast_node_printer (os_, indent_ + tabsize) (node);
    }

    tab (os_, indent_);
    os_ << '}';
    return os_;
  }

  std::ostream& operator() (value_array_t const& value) const
  {
    os_ << '[';
    bool first = true;
    BOOST_FOREACH (std::string const& v, value)
    {
      if (first) 
      {
        first = false;
        os_ << ' ';
      }
      else 
        os_ << ", ";

      os_ << v;
    }

    os_ << " ]";
    return os_;
  }

  std::ostream& operator() (value_single_t const& value) const
  {
    os_ << '"' << value << '"';
    return os_;
  }

  // binary TODO
  std::ostream& operator() (void const*) const
  {
    return os_;
  }

  std::ostream& operator() (value_void_t const&) const
  {
    return os_;
  }

  // %include
  std::ostream& operator() (boost::shared_ptr<ast> const& nodes) const
  {
    os_ << "// --------- included file ---------- //" << std::endl;
    BOOST_FOREACH (ast_node const& node, *nodes)
    {
      ast_node_printer (os_, indent_ + tabsize) (node);
    }

    os_ << "// ---------------------------------- //" << std::endl;
    return os_;
  }

  // %import
  std::ostream& operator() (ast const* nodes) const
  {
    os_ << "// --------- imported entries ------- //" << std::endl;
    BOOST_FOREACH (ast_node const& node, *nodes)
    {
      ast_node_printer (os_, indent_ + tabsize) (node);
    }

    os_ << "// ---------------------------------- //" << std::endl;
    return os_;
  }

  std::ostream& os_;
  int indent_;
};



inline std::ostream& 
operator<< (std::ostream& os, etype_t const& id)
{
  switch (id)
  {
    case Other: os << "other"; break;
    case Array: os << "array"; break;
    case Compound: os << "compound"; break;
    case Module: os << "module"; break;
    case Import: os << "import"; break;
    case Include: os << "include"; break;
    default: os << "***unknown***"; break;
  }
  return os;
} 

inline  
std::ostream& 
ast_node_printer::operator() (ast_node const& node) const
{
  tab (os_, indent_);
  os_ << node.line << '@';
  os_ << node.key << '(' << node.id << ')' << ' ';

  if (node.name) os_ << node.name.get () << ' ';

  boost::apply_visitor (ast_value_printer (os_, indent_), node.value);

  os_ << ';' << std::endl;

  return os_;
}

inline std::ostream&
operator<< (std::ostream& os, ast_value const& value)
{
  return boost::apply_visitor (ast_value_printer (os), value);
}

inline std::ostream&
operator<< (std::ostream& os, ast_node const& node)
{
  return ast_node_printer (os) (node);
}

inline std::ostream&
operator<< (std::ostream& os, ast const& av)
{
  BOOST_FOREACH (ast_node const& node, av)
  {
    os << node << std::endl;
  }

  return os;
}

YAMAIL_NS_DATA_CP_END
YAMAIL_NS_DATA_END
YAMAIL_NS_END

#endif // _YAMAIL_DATA_CONFIG_AST_IO_H_

#ifndef _YAMAIL_DATA_CONFIG_UTF8_PARSER_H_
#define _YAMAIL_DATA_CONFIG_UTF8_PARSER_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <string>

#include <boost/cstdint.hpp>

#include <boost/regex/pending/unicode_iterator.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/support_utree.hpp>

YAMAIL_FQNS_DATA_CP_BEGIN

namespace utf8 {

namespace qi = boost::spirit::qi;
namespace px = boost::phoenix;
namespace standard = boost::spirit::standard;

using boost::spirit::utree;
using boost::spirit::utf8_symbol_type;
using boost::spirit::utf8_string_type;
using boost::spirit::binary_string_type;

typedef boost::uint32_t uchar;

struct push_string 
{
  typedef void result_type;

  template <typename, typename>
  struct result 
  {
    typedef void type;
  };

  void operator() (std::string& s, uchar code_point) const
  {
    typedef std::back_insert_iterator<std::string> insert_iter;
    insert_iter out_iter(s);

    boost::utf8_output_iterator<insert_iter> s_iter(out_iter);
    *s_iter++ = code_point;
  }
}; // push_string

struct push_escaped_string 
{
  typedef void result_type;

  template <typename, typename>
  struct result 
  {
    typedef void type;
  };

  void operator() (std::string& s, uchar c) const
  {
    switch (c) {
      case 'b': 
        s += '\b';
        break;

      case 't': 
        s += '\t';
        break;

      case 'n': 
        s += '\n';
        break;

      case 'f': 
        s += '\f';
        break;

      case 'r': 
        s += '\r';
        break;

      case '"': 
        s += '"';
        break;

      case '\\': 
        s += '\\';
        break;
    }
  }
}; // push_escaped_string

template <typename Iterator>
struct parser: qi::grammar<Iterator, std::string ()>
{
  qi::rule<Iterator, void (std::string&)> escaped;
  qi::rule<Iterator, std::string ()> start;
  px::function<push_string> push_str;
  px::function<push_escaped_string> push_esc;

  parser () : parser::base_type (start)
  {
    using standard::char_;
    using qi::uint_parser;
    using qi::_val;
    using qi::_r1;
    using qi::_1;

    uint_parser<uchar, 16, 4, 4> hex4;
    uint_parser<uchar, 16, 8, 8> hex8;

    escaped
      = '\\'
      > (   ('u' >> hex4)           [push_str (_r1, _1)]
        |   ('U' >> hex8)           [push_str (_r1, _1)]
        |   char_ ("btnfr\\\"'")    [push_esc (_r1, _1)]
        );

    start
      = '"'
      > *(escaped (_val) | (~char_('"'))  [_val += _1])
      > '"';

    escaped.name("escaped_string");
    start.name("string");
  }
}; // struct parser

} // namespace utf8

YAMAIL_FQNS_DATA_CP_END

#endif // _YAMAIL_DATA_CONFIG_UTF8_PARSER_H_

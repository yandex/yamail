#ifndef _YAMAIL_DATA_CONFIG_COMMENTS_PARSER_H_
#define _YAMAIL_DATA_CONFIG_COMMENTS_PARSER_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <string>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

YAMAIL_NS_BEGIN
YAMAIL_NS_DATA_BEGIN
YAMAIL_NS_DATA_CP_BEGIN

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace standard = boost::spirit::standard;

template <typename Iterator>
struct comments_parser: qi::grammar<Iterator, std::string ()>
{
public:
  comments_parser () : comments_parser::base_type (comment) 
  {
    using qi::eol;
    using qi::char_;
    using qi::skip;
    using qi::lit;
    using qi::_val;
    using qi::_1;

    space_without_eol = ascii::space - eol;

    cpp_comment = skip (space_without_eol.alias ()) [
          lit ("//")
      >>  *(char_ - eol)
      >>  eol
    ];

    c_comment
      =   lit ("/*")
      >>  *(char_ - "*/")
      >>  "*/"
    ;
    

    comment = (cpp_comment | c_comment) [ _val = _1 ];

    space_without_eol.name ("space-without-eol");
    cpp_comment.name ("cpp-comment");
    c_comment.name ("c-comment");
    comment.name ("comment");
  }

  qi::rule<Iterator> space_without_eol;
  qi::rule<Iterator, std::string ()> cpp_comment;
  qi::rule<Iterator, std::string ()> c_comment, comment;

}; // comments_parser


template <typename Iterator>
struct whitespace : qi::grammar<Iterator> {

  qi::rule<Iterator> start;
  comments_parser<Iterator> comments;

  whitespace () : whitespace::base_type (start) 
  {
    using standard::space;

    start = space | comments;

    // BOOST_SPIRIT_DEBUG_NODE(space);
    // BOOST_SPIRIT_DEBUG_NODE(comments);
    BOOST_SPIRIT_DEBUG_NODE(start);
  }
}; // whitespace

YAMAIL_NS_DATA_CP_END
YAMAIL_NS_DATA_END
YAMAIL_NS_END
#endif // _YAMAIL_DATA_CONFIG_COMMENTS_PARSER_H_

#ifndef _YAMAIL_DATA_CONFIG_GRAMMAR_H_
#define _YAMAIL_DATA_CONFIG_GRAMMAR_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/data/config/utf8_parser.h>
#include <yamail/data/config/error_handler.h>
#include <yamail/data/config/comments_parser.h>
#include <yamail/data/config/ast.h>
#include <string>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/phoenix/core/argument.hpp> // phx::_3


#include <iostream>

BOOST_FUSION_ADAPT_STRUCT(
  YAMAIL_FQNS_DATA_CP::ast_node,
  (YAMAIL_FQNS_DATA_CP::etype_t, id)
  (std::string, key)
  (boost::optional<std::string>, name)
  (YAMAIL_FQNS_DATA_CP::ast_value, value)
  (std::size_t, line)
  (std::size_t, pos)
)

YAMAIL_FQNS_DATA_CP_BEGIN

namespace qi = boost::spirit::qi;
namespace px = boost::phoenix;
namespace standard = boost::spirit::standard;
namespace fusion = boost::fusion;
namespace phoenix = boost::phoenix;

using boost::spirit::utree;
using boost::spirit::utf8_symbol_type;
using boost::spirit::utf8_string_type;
using boost::spirit::binary_string_type;

struct save_line_pos
{
  template <typename, typename>
  struct result { typedef void type; };

  template <typename Range>
  void operator() (ast_node& ast_, Range const& rng) const
  {
    using boost::spirit::get_line;
    std::size_t n = get_line (rng.begin ());
    if (n != -1) 
    {
      BOOST_ASSERT (n <= (std::numeric_limits<short>::max)());
      ast_.line = n;
    } 
    else
      ast_.line = -1;
  }
};

template <typename Iterator, typename F>
struct tagger: qi::grammar<Iterator, void (ast_node&/*, char*/)>
{
  qi::rule<Iterator, void (ast_node&/*, char*/)> start;
  qi::rule<Iterator, void(ast_node&)> epsilon;

  px::function<F> f;

  tagger (F f_ = F ()) : tagger::base_type (start), f (f_)
  {
    using qi::omit;
    using qi::raw;
    using qi::eps;
    using qi::lit;
    using qi::_1;
    using qi::_r1;
    using qi::_r2;

    // start   = omit[raw[lit(_r2)] [f(_r1, _1)]];
    start   = omit[raw[eps]      [f(_r1, _1)]];
    epsilon = omit[raw[eps]      [f(_r1, _1)]];
  }
}; // tagger

template <typename Iterator>
struct multi_string_parser
  : qi::grammar<Iterator, std::string (), whitespace<Iterator> >
{
  typedef whitespace<Iterator> wspc; 

  qi::rule<Iterator, std::string (), wspc> multi_string, string, string_pair,
    string_seq, any_string, qstring_pair;

  utf8::parser<Iterator> quoted_string;

  multi_string_parser () : multi_string_parser::base_type (multi_string)
  {
    using qi::lexeme;
    using qi::char_;
    using qi::_val;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;

    string = lexeme [ +(char_ - char_ (" ;\"][{}")) ] ;

    // multi_string = ( string_seq | quoted_string | string );
    multi_string = string_seq.alias ();

    any_string = string | quoted_string;

    qstring_pair = 
          (quoted_string >> qstring_pair  ) [ _val = _1 +       _2 ]
      |   (quoted_string >> string_pair   ) [ _val = _1 + " " + _2 ]
      |   (quoted_string >> quoted_string ) [ _val = _1 +       _2 ]
      |   (quoted_string >>        string ) [ _val = _1 + " " + _2 ]
    ;

    string_pair = 
          (string >> qstring_pair)        [ _val = _1 + " " + _2 ]
      |   (string >>  string_pair)        [ _val = _1 + " " + _2 ]
      |   (string >>   any_string)        [ _val = _1 + " " + _2 ]
    ;

    string_seq = 
      string_pair 
    | qstring_pair
    | any_string 
    ;

    BOOST_SPIRIT_DEBUG_NODE(multi_string);
    BOOST_SPIRIT_DEBUG_NODE(string);
    BOOST_SPIRIT_DEBUG_NODE(string_pair);
    BOOST_SPIRIT_DEBUG_NODE(qstring_pair);
    BOOST_SPIRIT_DEBUG_NODE(any_string);
    BOOST_SPIRIT_DEBUG_NODE(string_seq);
  }
};

template <typename Iterator, typename ErrorHandler>
struct null_include
{
  template <typename, typename=void> 
  struct result { typedef void type; };

  ErrorHandler error;

  null_include (ErrorHandler error)
    : error (error)
  {
  }

  void
  operator() (ast_node& node, std::string const& file) const
  {
    std::cout << "include: " << file << std::endl;
  }
};


template <
    typename Iterator
  , typename ErrorHandler = error_handler<>
  , typename IncludeHandler = null_include<Iterator, ErrorHandler>
>
struct grammar: qi::grammar<Iterator, ast (), whitespace<Iterator> >
{
  typedef whitespace<Iterator> wspc; 

  qi::rule<Iterator, ast (), wspc> start, statements;
  qi::rule<Iterator, ast_node (), wspc> statement;

  qi::rule<Iterator, std::string (), wspc> s;
  multi_string_parser<Iterator> multi_string;

  qi::rule<Iterator, ast (), wspc> compound_value;

  qi::rule<Iterator, value_array_t (), wspc> array;

  qi::rule<Iterator, ast_value (etype_t&, boost::optional<std::string>&), wspc> value;
  qi::rule<Iterator, ast_value (etype_t&), wspc> value_var;

  qi::rule<Iterator, std::string ()> string, single_string, key, name;

  qi::rule<Iterator, ast_node (), wspc> control_command, regular_command;

  utf8::parser<Iterator> quoted_string;

  px::function<ErrorHandler> const error;
  px::function<IncludeHandler> const include;

  tagger<Iterator, save_line_pos> pos;

  explicit
  grammar (std::string const& source = "<source>") 
    : grammar::base_type (start)
    , error (ErrorHandler (source))
    , include (ErrorHandler (source))
  { 
    init ();
  }
    
  explicit 
  grammar (ErrorHandler ehandler)
    : grammar::base_type (start)
    , error (ehandler)
    , include (ehandler)
  {
    init ();
  }
    
  explicit
  grammar (IncludeHandler ihandler, std::string const& source = "<source>")
    : grammar::base_type (start)
    , error (ErrorHandler (source))
    , include (ihandler)
  {
    init ();
  }

  grammar (std::string const& source, IncludeHandler ihandler)
    : grammar::base_type (start)
    , error (ErrorHandler (source))
    , include (ihandler)
  {
    init ();
  }
    
   grammar (ErrorHandler ehandler, IncludeHandler ihandler)
    : grammar::base_type (start)
    , error (ehandler)
    , include (ihandler)
  {
    init ();
  }

   void init ();
};

template <typename Iterator, typename ErrorHandler, typename IncludeHandler>
void 
grammar<Iterator, ErrorHandler, IncludeHandler>::init ()
{
  using qi::lexeme;
  using qi::no_case;
  using qi::char_;
  using qi::eps;
  using qi::lit;
  using qi::_val;
  using qi::on_error;
  using qi::retry;
  using qi::_r1;
  using qi::_r2;
  using qi::_1;
  using qi::_2;
  using qi::_3;
  using qi::_4;

  using phoenix::at_c;
  using phoenix::push_back;
  using phoenix::val;

  namespace phx_arg = phoenix::arg_names;

  start = statements.alias ();
  // statements = *statement [ push_back (at_c<2> (_val), _1) ];
  statements = *statement;

  statement = pos(_val) >> (control_command | regular_command) > ';';

  control_command 
    =   // pos (_val, '%') 
        lit ('%')
    >   ( 
          (   qi::string ("import")   [ at_c<AST_ID> (_val) = val (Import) ]
//                                      [ at_c<AST_KEY> (_val) = val ("import") ]
            > single_string           [ at_c<AST_NAME> (_val) = _1 ]
          )
        | (   qi::string ("include")  [ at_c<AST_ID> (_val) = val (Include) ]
//                                      [ at_c<AST_KEY> (_val) = val ("include") ]
                                      // put included filename into name field
            > single_string           [ at_c<AST_NAME> (_val) = _1 ] 
                                      [ include (_val, _1) ]
          )
        )  
    >   -(    no_case ["into"] 
            > multi_string            [ at_c<AST_KEY> (_val) = _1 ]
         )
    ;

  regular_command 
    =   eps                                   [ at_c<AST_ID> (_val) = val (Other) ]
    >>  key                                   [ at_c<AST_KEY> (_val) = _1 ]
    >   value(at_c<AST_ID> (_val), at_c<AST_NAME> (_val)) [ at_c<AST_VALUE> (_val) = _1 ]
  ;

  string = +(char_ - char_ (" ;\"][{}")) ;


  single_string = (quoted_string | string);
  key = name = single_string;

  value = 
    (     name            [ _r2 = _1 ]  [ _r1 = val (Compound) ]
      >>  compound_value  [ _val = _1 ]
    ) 
    | value_var(_r1)      [ _r2 = val(boost::optional<std::string> ()) ]
                          [ _val = _1 ]
  ;


  array = // pos (_val, '[') 
      lit ('[') 
    > *(quoted_string | string) 
    > ']'
  ;

  compound_value = // pos (_val, '{') 
      lit ('{') 
    > statements
    > '}'
  ;

  value_var 
    %=  compound_value [ _r1 = val (Compound) ] 
    |   array          [ _r1 = val (Array)    ]
    |   multi_string   [ _r1 = val (Other)    ]
  ;

  BOOST_SPIRIT_DEBUG_NODE(statement);
  BOOST_SPIRIT_DEBUG_NODE(statements);
  BOOST_SPIRIT_DEBUG_NODE(start);
  BOOST_SPIRIT_DEBUG_NODE(string);
  BOOST_SPIRIT_DEBUG_NODE(single_string);
  BOOST_SPIRIT_DEBUG_NODE(key);
  BOOST_SPIRIT_DEBUG_NODE(name);
  BOOST_SPIRIT_DEBUG_NODE(value);
  BOOST_SPIRIT_DEBUG_NODE(array);
  BOOST_SPIRIT_DEBUG_NODE(compound_value);
  BOOST_SPIRIT_DEBUG_NODE(value_var);
  BOOST_SPIRIT_DEBUG_NODE(control_command);
  BOOST_SPIRIT_DEBUG_NODE(regular_command);

  on_error<retry> (start, error (phx_arg::_3, _1, _2, _3, _4));
}

YAMAIL_FQNS_DATA_CP_END

#endif // _YAMAIL_DATA_CONFIG_GRAMMAR_H_

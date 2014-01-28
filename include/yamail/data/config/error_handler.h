#ifndef _YAMAIL_DATA_CONFIG_ERROR_HANDLER_H_
#define _YAMAIL_DATA_CONFIG_ERROR_HANDLER_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <string>
#include <sstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/home/support/info.hpp>
#include <boost/spirit/include/support_line_pos_iterator.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <yamail/data/config/detail/line_iterator_fixed.h>

YAMAIL_FQNS_DATA_CP_BEGIN

using boost::spirit::info;

template <typename Out>
struct print_info 
{
  typedef boost::spirit::utf8_string string;

  print_info (Out& out) : out (out), first (true) {}

  void element (string const& tag, string const& value, int) const
  {
    if (first) 
      first = false;
    else
      out << ' ';
    

    if (value == "") 
      out << tag;
    else
      out << '"' << value << '"';
  }

  Out& out;
  mutable bool first;
}; // print_info

struct expected_component : std::exception 
{ 
  std::string msg;
  std::string current_line;
  std::size_t column;

  template <typename Iterator>
  expected_component (std::string const& source, std::size_t line, 
      info const& w, boost::iterator_range<Iterator> const& cline,
      std::size_t col)
  : current_line (cline.begin (), cline.end ())
  , column (col)
  {
    using boost::spirit::basic_info_walker;

    std::ostringstream oss;
    oss << "(exception \"" << source << "\" ";

    if (line == -1) oss << -1;
    else            oss << line;

    oss << " '(expected_component (";

    print_info <std::ostringstream> pr (oss);
    basic_info_walker<print_info<std::ostringstream> > walker(pr, w.tag, 0);

    boost::apply_visitor(walker, w.value);

    oss << ")))";

    msg = oss.str ();
  }

  virtual ~expected_component() throw() {}

  virtual char const* what() const throw()
  {
    return msg.c_str ();
  }
}; // expected_component

namespace qi = boost::spirit::qi;

struct default_custom_error_handler
{
  template <typename Iterator>
  bool operator() (std::string const& source, info const& w, 
      std::size_t line_n, std::size_t col,
      boost::iterator_range<Iterator> const& cline) const
  {
    throw expected_component(source, line_n, w, cline, col);
    return false;
  }
};

template <typename CustomHandler = default_custom_error_handler>
struct error_handler
{
  template <typename, typename, typename, typename, typename>
  struct result
  {
    typedef void type;
  };

  std::string source_;
  CustomHandler* custom_;
  boost::shared_ptr<CustomHandler> custom_own_;

  void custom_init (CustomHandler& custom) 
  {
    custom_ = &custom;
  }

  void custom_init (CustomHandler const& custom)
  {
    custom_own_ = boost::make_shared<CustomHandler> (custom);
    custom_ = custom_own_.get ();
  }

#if __cplusplus >= 201103L
  void custom_init (CustomHandler&& custom) 
  {
    custom_ = &custom;
  }
#endif // C++11

  error_handler(std::string const& source = "<string>",
      CustomHandler const& custom = CustomHandler ()) 
    : source_ (source)
  { custom_init (custom); }

  error_handler(CustomHandler const& custom,
      std::string const& source = "<string>")
    : source_ (source)
  { custom_init (custom); }

  error_handler(std::string const& source, CustomHandler& custom) 
    : source_ (source)
  { custom_init (custom); }

  error_handler(CustomHandler& custom,
      std::string const& source = "<string>")
    : source_ (source)
  { custom_init (custom); }

#if __cplusplus >= 201103L
  error_handler(std::string const& source,
      CustomHandler&& custom) 
    : source_ (source)
  { custom_init (custom); }

  error_handler(CustomHandler&& custom,
      std::string const& source = "<string>")
    : source_ (source)
  { custom_init (custom); }
#endif

  template <typename Iterator>
  void operator()(qi::error_handler_result& action, 
      Iterator first, Iterator last, Iterator err_pos,
      info const& what) const
  {
    using boost::spirit::get_line;
    using qi::retry;
    using qi::fail;

    Iterator eol = err_pos;
    std::size_t line_n = get_line(err_pos);

    boost::iterator_range<Iterator> line = 
      detail::get_current_line_FIXED (first, err_pos, last);

    std::size_t column = detail::get_column_FIXED (line.begin (), err_pos);

    // std::cerr << "string with error: " << line << "\n";
    // throw expected_component(source, line_n, what, line, column);
    
    action = (*custom_) (source_, what, line_n, column, line) ? retry : fail;
  }
};

YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_ERROR_HANDLER_H_

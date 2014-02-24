#ifndef _YAMAIL_DATA_CONFIG_ERROR_WRAPPER_H_
#define _YAMAIL_DATA_CONFIG_ERROR_WRAPPER_H_

#include <yamail/config.h>
#include <yamail/data/config/namespace.h>

#include <yamail/compat/shared_ptr.h>


#include <string>
#include <sstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/home/support/info.hpp>
#include <boost/spirit/include/support_line_pos_iterator.hpp>

#include <yamail/data/config/detail/line_iterator_fixed.h>

YAMAIL_FQNS_DATA_CP_BEGIN

namespace detail {

namespace spirit = boost::spirit;
namespace qi = spirit::qi;

template <typename Out>
struct print_info 
{
  typedef boost::spirit::utf8_string string;

  print_info (Out& out) : out (out), first (true) {}

  void element (string const& tag, string const& value, int) const
  {
    if (first) first = false;
    else out << ' ';
    

    if (value == "") out << tag;
    else out << '"' << value << '"';
  }

  Out& out;
  mutable bool first;
}; // print_info

} // namespace detail

struct expected_component : std::exception 
{ 
  std::string msg;
  std::string current_line;
  std::size_t column;

  template <typename Iterator>
  expected_component (std::string const& source, std::size_t line, 
      boost::spirit::info const& w, 
      boost::iterator_range<Iterator> const& cline,
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

    detail::print_info <std::ostringstream> pr (oss);
    basic_info_walker<detail::print_info<std::ostringstream> > 
      walker(pr, w.tag, 0);

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


struct default_error_handler
{
  template <typename Iterator>
  bool operator() (
      std::string const& source, boost::spirit::info const& w, 
      std::size_t line_n, std::size_t col,
      boost::iterator_range<Iterator> const& cline) const
  {
    throw expected_component(source, line_n, w, cline, col);
    return false;
  }
};

template <typename ErrorHandler = default_error_handler>
struct error_wrapper
{
  // for phoenix v3
  typedef void result_type;

  // for phoenix v2
  template <typename, typename, typename, typename, typename>
  struct result
  {
    typedef void type;
  };

  std::string source_;
  ErrorHandler* error_handler_;
  compat::shared_ptr<ErrorHandler> error_handler_own_;

  // if reference - use it inplace. otherwise - copy.
  void error_handler_init (ErrorHandler& error_handler) 
  {
    error_handler_ = &error_handler;
  }

  void error_handler_init (ErrorHandler const& error_handler)
  {
    error_handler_own_ = compat::make_shared<ErrorHandler> (error_handler);
    error_handler_ = error_handler_own_.get ();
  }

  void error_handler_init (ErrorHandler&& error_handler) 
  {
    error_handler_own_ = 
      compat::make_shared<ErrorHandler> (std::move (error_handler));

    error_handler_ = error_handler_own_.get ();
  }

  template <typename EH = ErrorHandler, class = std::enable_if<
    std::is_same<ErrorHandler, typename std::decay<EH>::type>::value> >
  error_wrapper(
      std::string const& source = "<string>",
      EH&& error_handler = ErrorHandler ()) 
    : source_ (source)
  { error_handler_init (std::forward<EH> (error_handler)); }

  template <typename EH = ErrorHandler, class = std::enable_if<
    std::is_same<ErrorHandler, typename std::decay<EH>::type>::value> >
  error_wrapper(
      ErrorHandler&& error_handler,
      std::string const& source = "<string>")
    : source_ (source)
  { error_handler_init (std::forward<EH> (error_handler)); }

  template <typename Iterator>
  void operator() (
      boost::spirit::qi::error_handler_result& action, 
      Iterator first, Iterator last, Iterator err_pos,
      boost::spirit::info const& what) const
  {
    using boost::spirit::get_line;
    using boost::spirit::qi::retry;
    using boost::spirit::qi::fail;

    Iterator eol = err_pos;
    std::size_t line_n = get_line(err_pos);

    boost::iterator_range<Iterator> line = 
      detail::get_current_line_FIXED (first, err_pos, last);

    std::size_t column = detail::get_column_FIXED (line.begin (), err_pos);

    // std::cerr << "string with error: " << line << "\n";
    // throw expected_component(source, line_n, what, line, column);
    
    action = (*error_handler_) (source_, what, line_n, column, line) 
      ? retry : fail;
  }
};

YAMAIL_FQNS_DATA_CP_END
#endif // _YAMAIL_DATA_CONFIG_ERROR_WRAPPER_H_

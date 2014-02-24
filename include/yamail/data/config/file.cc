#define BOOST_SPIRIT_DEBUG_PRINT_SOME 80

#include <yamail/data/config/parse_file.h>
#include <yamail/data/config/parse_stream.h>
#include <yamail/data/config/ast_io.h>

#include <iostream>
#include <boost/shared_ptr.hpp>

class my_error_handler 
{
public:
  my_error_handler () : errors_ (0) {}

  template <typename Iterator>
  bool operator() (std::string const& source, boost::spirit::qi::info const& w, 
      std::size_t line_n, std::size_t col_n,
      boost::iterator_range<Iterator> const& line)
  {
    using boost::spirit::basic_info_walker;
    std::cerr << source << ':' << line_n << ':' << col_n << ": expected ";
    print_info<std::ostream> pr (std::cerr);
    basic_info_walker<print_info<std::ostream> > walker(pr, w.tag, 0);
    boost::apply_visitor (walker, w.value);
    std::cerr << '\n';

    if (++errors_ > 5) {
      std::cerr << source << ':' << line_n << ':' << col_n 
        << ": parsing stopped because of too many errors.";
      return false;
    }

    return true;
  }
private:
  template <typename Out>
  struct print_info 
  {
    Out& out;
    mutable bool first;
    typedef boost::spirit::utf8_string string;
    print_info (Out& out) : out (out), first (true) {}

    void element (string const& tag, string const& value, int) const {
      if (first) first = false;
      else       out << ' ';
  
      if (value == "") out << tag;
      else             out << '"' << value << '"';
    }
  };

  std::size_t errors_;
};

int main (int ac, char *av[])
{
  namespace spirit = boost::spirit;
  namespace qi = spirit::qi;
  namespace config = y::data::config;

  bool r;
  config::ast data;

  my_error_handler my_eh;
  
  try {
    if (ac == 1)
      r = config::parse (data, std::cin, my_eh);
    else
      r = config::parse_file (data, av[1], my_eh);
  } 
  catch (config::expected_component const& e)
  {
    std::cout << "-------------------------\n";
    std::cout << "Parsing failed:\n";
    std::cout << "-------------------------\n";
#if 0
    std::string rest(iter, end);
    std::cout << "Current line:\n" << e.current_line << "\n";
    std::cout << std::string (e.column-1, ' ') << "^\n";
    std::cout << std::string (e.column-1, ' ') << "|\n";
    std::cout << std::string (e.column-1, ' ') << e.what () << "\n";
#endif
    return 0;
  }

  if (r)
  {
    std::cout << "-------------------------\n";
    std::cout << "Parsing succeeded\n";
    std::cout << "-------------------------\n";
    std::cout << data << "\n";
  }
  else
  {
    std::cout << "-------------------------\n";
    std::cout << "Parsing failed\n";
#if 0
    std::string rest(iter, end);
    std::cout << "stopped at: \": " << rest << "\"\n";
#endif
    std::cout << "-------------------------\n";
  }
}

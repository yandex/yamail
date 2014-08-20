#ifndef _YAMAIL_LOG_DSV_BACKEND_H_
#define _YAMAIL_LOG_DSV_BACKEND_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>

#include <boost/phoenix.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/attributes/value_visitation.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace log {
#else
YAMAIL_FQNS_LOG_BEGIN
#endif // GENERATING_DOCUMENTATION
namespace typed {

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;


template <typename C, typename T>
struct print_visitor 
{
	std::basic_ostream<C,T>& os_;
	print_visitor (std::basic_ostream<C,T>& os) : os_ (os) {}
	typedef void result_type;
	template <typename X>
	result_type operator() (X const& x) const
	{
		os_ << x;
  }
};

template <typename C, typename T>
void print_value (std::basic_ostream<C,T>& os, logging::attribute_value const& attr)
{
  typedef boost::mpl::vector<int, std::string, unsigned int> types;
  // logging::visitation_result result = 
    logging::visit< types >(attr, print_visitor<C,T>(os));
}



template <typename CharT = char, typename Traits = std::char_traits<CharT> >
class basic_dsv_sink_backend:
  public sinks::basic_sink_backend<
    sinks::combine_requirements<
      sinks::synchronized_feeding,
      sinks::flushing
    >::type
  >
{
	typedef CharT char_type;

private:
  char_type delim_char_;
  bool      escaping_;
  char_type escape_char_;

  std::basic_ostream<CharT, Traits>& file_;

public:
  explicit basic_dsv_sink_backend (std::basic_ostream<CharT, Traits>& file,
      char_type delim = ',')
    : delim_char_ (delim)
    , escaping_ (false)
    , escape_char_ ('\0')
    , file_ (file)
  {
  }

  explicit basic_dsv_sink_backend (std::basic_ostream<CharT, Traits>& file,
      char_type delim, char_type escape_char)
    : delim_char_ (delim)
    , escaping_ (true)
    , escape_char_ (escape_char)
    , file_ (file)
  {
  }

  void consume (logging::record_view const& rec)
  {
    for (auto const& x: rec.attribute_values())
    {
    	file_ << x.first << "=";
    	print_value (file_, x.second);
    	file_ << delim_char_;
    }
  }

  void flush ()
  {
  	file_.flush ();
  }

private:
  void write_data ()
  {
  	;
  }
};

typedef basic_dsv_sink_backend<> dsv_sink_backend;


} // namespace typed
#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_LOG_DSV_BACKEND_H_

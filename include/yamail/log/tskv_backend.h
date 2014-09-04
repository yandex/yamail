#ifndef _YAMAIL_LOG_TSKV_BACKEND_H_
#define _YAMAIL_LOG_TSKV_BACKEND_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>

#include <yamail/log/typed.h>

#include <boost/phoenix.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/attributes/value_visitation.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>

#include <boost/chrono/chrono_io.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/foreach.hpp>

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

BOOST_LOG_ATTRIBUTE_KEYWORD_TYPE (tskv_attributes, 
    "tskv_attributes",
    y::log::typed::attributes_map)

BOOST_LOG_ATTRIBUTE_KEYWORD (tskv_format, 
    "tskv_format",
    std::string)

template <typename CharT, typename Traits, typename DelimCharT>
class print_visitor 
{
	std::basic_ostream<CharT,Traits>& os_;
	logging::attribute_name const& name_;
	DelimCharT delim_;

public:
	print_visitor (std::basic_ostream<CharT,Traits>& os,
	  logging::attribute_name const& name, 
	  DelimCharT delim) : os_ (os), name_ (name), delim_ (delim)
	{}

	typedef void result_type;

	template <typename X> result_type operator() (X const& x) const
	{
		os_ << name_ << '=' << x;
  }

  result_type operator() (y::log::typed::attributes_map const& x) const
	{
		// os_ << x;
		// os_ << "<HERE>";

    bool first = true;
		BOOST_FOREACH (attr_name const& aname, cascade_keys (x))
		{
			if (boost::optional<attr_value const&> aval = cascade_find (x, aname))
				if (! is_deleted (*aval))
        {
          if (first) first = false;
          else os_ << '\t';

          os_ << aname << '=' << *aval;
        }
    }
  }
};

template <typename CharT, typename Traits, typename DelimCharT>
void print_value (std::basic_ostream<CharT,Traits>& os, 
  logging::attribute_name const& name, 
  logging::attribute_value const& attr, DelimCharT delim_char)
{
  typedef boost::mpl::vector<int, std::string, unsigned int,
      y::log::typed::attributes_map> types;
  // logging::visitation_result result = 
    logging::visit<types> (attr, 
      print_visitor<CharT,Traits,DelimCharT>(os, name, delim_char));
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT,Traits>&
operator<< (std::basic_ostream<CharT,Traits>& os, 
    y::log::typed::attributes_map const& map)
{
  bool first = true;

  BOOST_FOREACH (attr_name const& aname, cascade_keys (map))
  {
  	std::cout << "name = " << aname << "\n";
    if (boost::optional<attr_value const&> aval = cascade_find (map, aname))
      if (! is_deleted (*aval))
      {
        os << '\t' << aname << '=' << *aval;
      }
  }
	return os;
}


template <typename CharT = char, typename Traits = std::char_traits<CharT> >
class basic_tskv_sink_backend:
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
  std::basic_ostream<CharT, Traits>& file_;

public:
  explicit basic_tskv_sink_backend (std::basic_ostream<CharT, Traits>& file,
      char_type delim = '\t')
    : delim_char_ (delim)
    , file_ (file)
  {
  }

  explicit basic_tskv_sink_backend (std::basic_ostream<CharT, Traits>& file,
      char_type delim, char_type escape_char)
    : delim_char_ (delim)
    , file_ (file)
  {
  }

  void consume (logging::record_view const& rec)
  {
  	file_ << "tskv\t";
  	bool first = true;
    BOOST_FOREACH (logging::attribute_value_set::value_type const& x,
        rec.attribute_values())
    {
    	// file_ << x.first << "=";
    	if (first) first = false;
    	else file_ << '\t';
    	print_value (file_, x.first, x.second, delim_char_);
    }

    file_ << "\n";
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

typedef basic_tskv_sink_backend<> tskv_sink_backend;

template <typename CharT, typename Traits>
inline void
init_tskv_backend2 (std::basic_ostream<CharT, Traits>& os)
{
	typedef basic_tskv_sink_backend<CharT,Traits> tskv_backend;
	typedef sinks::synchronous_sink< tskv_backend > sink_t;

	boost::shared_ptr< logging::core > core = logging::core::get();
	boost::shared_ptr< tskv_backend > backend(new tskv_backend (os));
	boost::shared_ptr< sink_t > sink(new sink_t(backend));

	sink->set_filter (expr::has_attr (tskv_format));
	core->add_sink(sink);
}

inline void
init_tskv_backend (std::string const& file_mask)
{
	boost::shared_ptr< logging::core > core = logging::core::get();

  boost::shared_ptr< sinks::text_file_backend > backend =
      boost::make_shared< sinks::text_file_backend >(
          keywords::file_name = file_mask,
          keywords::rotation_size = 5 * 1024 * 1024
      );

  // Wrap it into the frontend and register in the core.
  // The backend requires synchronization in the frontend.
  typedef sinks::synchronous_sink< sinks::text_file_backend > sink_t;
  boost::shared_ptr< sink_t > sink(new sink_t(backend));

  sink->set_formatter
  (
    expr::format("tskv\ttskv_format=%1%%2%")
      % expr::attr< std::string > ("tskv_format")
      % expr::attr< y::log::typed::attributes_map > ("tskv_attributes")
  );

	sink->set_filter (expr::has_attr (tskv_format));
	core->add_sink(sink);
}

} // namespace typed
#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_LOG_TSKV_BACKEND_H_

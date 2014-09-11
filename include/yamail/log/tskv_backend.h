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
#include <boost/log/utility/setup/formatter_parser.hpp>

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

#if 1
BOOST_LOG_ATTRIBUTE_KEYWORD (tskv_attributes, 
    "tskv_attributes",
    y::log::typed::attributes_map)
BOOST_LOG_ATTRIBUTE_KEYWORD (tskv_wattributes, 
    "tskv_wattributes",
    y::log::typed::wattributes_map)
#endif

BOOST_LOG_ATTRIBUTE_KEYWORD (tskv_format, 
    "tskv_format",
    std::string)

template <typename CharT, typename Traits, typename Alloc>
std::basic_ostream<CharT,Traits>&
operator<< (std::basic_ostream<CharT,Traits>& os, 
    y::log::typed::basic_attributes_map<CharT,Traits,Alloc> const& map)
{
  bool first = true;

  typedef typename attr<CharT,Traits,Alloc>::name attr_name;
  BOOST_FOREACH (attr_name const& aname, cascade_keys (map))
  {
    if (boost::optional<typename attr<CharT,Traits,Alloc>::value const&> aval =
    	   cascade_find (map, aname))
      if (! is_deleted (*aval))
      {
        os << '\t' << aname << '=' << *aval;
      }
  }
	return os;
}

template <typename CharT, typename Traits, typename Alloc>
class tskv_formatter
{
public:
  typedef void result_type;

public:
  typedef std::basic_string<CharT, Traits, Alloc> string_type;

  explicit tskv_formatter (string_type const& fmt)
    : format_ (fmt)
  {
  }

#if 0
  template <typename CharT, typename Traits2, typename Alloc2>
  void operator() (
      logging::basic_formatting_ostream<CharT,Traits2,Alloc2>& strm,
      logging::value_ref<basic_attributes_map<CharT,Traits2,Alloc2> const& value) const
#else
	template <typename Stream, typename Traits2, typename Alloc2>
	void operator() (Stream& strm, 
    logging::value_ref<basic_attributes_map<CharT,Traits2,Alloc2> > const& 
      value) const
#endif
	{
		if (value)
    {
    	basic_attributes_map<CharT,Traits2,Alloc2> const& map = value.get ();
    	strm << map;
    }
  }

private:
  string_type format_;
};

template <typename CharT, typename Traits, typename Alloc>
tskv_formatter<CharT,Traits,Alloc>
make_tskv_formatter (std::basic_string<CharT,Traits,Alloc> const& str)
{
	return tskv_formatter<CharT,Traits,Alloc> (str);
}

template <
    typename CharT
  , typename Traits = std::char_traits<CharT>
  , typename Alloc = std::allocator<CharT>
>
class tskv_formatter_factory
    : public logging::basic_formatter_factory<
          CharT
        , basic_attributes_map<CharT,Traits,Alloc> 
      >
{
	static CharT const delimiter_[];

public:
  typedef tskv_formatter_factory self_t;

  typename self_t::formatter_type
  create_formatter (logging::attribute_name const& name,
      typename self_t::args_map const& args)
  {

  	typename self_t::args_map::const_iterator it = args.find (delimiter_);

  	if (it != args.end ())
    {
    	return boost::phoenix::bind (
    	      make_tskv_formatter (it->second), expr::stream,
    	      expr::attr<basic_attributes_map<CharT,Traits,Alloc> > (name)
    	);
    }
    else
    {
    	return expr::stream 
    	    << expr::attr< basic_attributes_map<CharT,Traits,Alloc> >(name);
    }
  }
};

// works for char and wchar_t
template <typename C, typename T, typename A>
C const tskv_formatter_factory<C,T,A>::delimiter_[] =
{ 'd', 'e', 'l', 'i', 'm', 'i', 't', 'e', 'r', '\0' };

namespace detail {

class tskv_formatter_init_helper
{
private:
	tskv_formatter_init_helper ()
	{
#if 1
		logging::register_formatter_factory ("tskv_attributes",
		  boost::make_shared<tskv_formatter_factory<char> > ());
#endif
#if 1
		logging::register_formatter_factory ("tskv_wattributes",
		  boost::make_shared<tskv_formatter_factory<char> > ());
#endif
  }

public:
  inline static tskv_formatter_init_helper&
  get_instance ()
  {
    static tskv_formatter_init_helper instance;
    return instance;
  }
};

static const tskv_formatter_init_helper&
    tskv_formatter_init_helper_ = tskv_formatter_init_helper::get_instance ();

} // namespace detail

} // namespace typed
#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_LOG_TSKV_BACKEND_H_

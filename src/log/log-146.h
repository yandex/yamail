#include <yamail/log/log.h>

#include <fstream>
#include <map>

#ifndef SYSLOG_NAMES
#define SYSLOG_NAMES
#include <sys/syslog.h>
#undef SYSLOG_NAMES
#else
#include <sys/syslog.h>
#endif

#include <yamail/log/rotate_text_file_backend.h>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/static_assert.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/utility/init/to_console.hpp>

#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sinks/text_multifile_backend.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>

#include <boost/log/formatters/basic_formatters.hpp>
#include <boost/log/formatters/attr.hpp>
#include <boost/log/formatters/format.hpp>
#include <boost/log/formatters/named_scope.hpp>
#include <boost/log/formatters/message.hpp>
#include <boost/log/formatters/date_time.hpp>
#include <boost/log/formatters/stream.hpp>

#include <boost/log/filters/attr.hpp>
#include <boost/log/filters/has_attr.hpp>

#include <boost/log/attributes/named_scope.hpp>

#include <boost/log/utility/init/common_attributes.hpp>
#include <boost/log/utility/init/from_stream.hpp>
#include <boost/log/utility/init/from_settings.hpp>

YAMAIL_FQNS_LOG_BEGIN

BOOST_LOG_GLOBAL_LOGGER_CTOR_ARGS(global_logger, logger_t, (boost::log::keywords::channel = "general"))

namespace logging = boost::log;
namespace fmt = boost::log::formatters;
namespace filters = boost::log::filters;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;

static const char* levels[] =
{ "emerg", "alert", "fatal", "error", "warn", "notice", "info", "debug" };

static const std::size_t levels_size = sizeof (levels) / sizeof (*levels);

static const char* levels_formatted[] =
{
    "emerg ",
    "alert ",
    "fatal ",
    "error ",
    "warn  ",
    "notice",
    "info  ",
    "debug "
};

BOOST_STATIC_ASSERT(levels_size == (sizeof (levels_formatted) / sizeof (*levels_formatted)));
BOOST_STATIC_ASSERT(levels_size == static_cast<size_t>(end_of_sev_level));

boost::shared_ptr< sinks::sink< char > >
create_multifile_sink(std::map< std::string, boost::any > const& params)
{
    // Read parameters for the backend and create it
    std::map< std::string, boost::any >::const_iterator dir_par = params.find("Directory");
    std::map< std::string, boost::any >::const_iterator attr_par = params.find("AttrName");
    if (dir_par == params.end())
        throw std::runtime_error("Directory parameter not specified for the MultiFile backend");
    if (attr_par == params.end())
        throw std::runtime_error("AttrName parameter not specified for the MultiFile backend");

    boost::shared_ptr< sinks::text_multifile_backend > backend =
        boost::make_shared< sinks::text_multifile_backend >();

    backend->set_file_name_composer(fmt::stream << fmt::fmt_wrapper< char, std::string >(boost::any_cast<std::string>(dir_par->second))
                                                << fmt::attr< std::string >(boost::any_cast<std::string>(attr_par->second))
                                                << ".log");

    // Construct and initialize the final sink
    boost::shared_ptr< sinks::synchronous_sink< sinks::text_multifile_backend > > sink =
        boost::make_shared< sinks::synchronous_sink< sinks::text_multifile_backend > >(backend);

    std::map< std::string, boost::any >::const_iterator it = params.find("Filter");
    if (it != params.end())
        sink->set_filter(logging::parse_filter(boost::any_cast<std::string>(it->second)));

    return sink;
}

boost::shared_ptr< sinks::sink< char > >
create_rotate_text_file_sink(std::map< std::string, boost::any > const& params)
{
  typedef std::basic_istringstream< char > isstream;
  typedef rotate_basic_text_file_backend< char > backend_t;
  typedef std::map< std::string, boost::any > params_t;
  boost::shared_ptr< backend_t > backend = boost::make_shared< backend_t >();

  // FileName
  params_t::const_iterator it =
      params.find("FileName");
  if (it != params.end())
    backend->set_file_name(boost::any_cast<std::string>(it->second));
  else
    boost::throw_exception(std::runtime_error("File name is not specified"));

  // Auto flush
  it = params.find("AutoFlush");
  if (it != params.end())
  {
    bool f = false;
    isstream strm(boost::any_cast<std::string>(it->second));
    strm.setf(std::ios_base::boolalpha);
    strm >> f;
    backend->auto_flush(f);
  }

  // Filter
  typedef sinks::sink< char >::filter_type filter_type;
  filter_type filt;
  it = params.find("Filter");
  if (it != params.end())
  {
    filt = logging::parse_filter(boost::any_cast<std::string>(it->second));
  }

  // Formatter
  it = params.find("Format");
  if (it != params.end())
  {
    backend->set_formatter(logging::parse_formatter(boost::any_cast<std::string>(it->second)));
  }

  boost::shared_ptr< sinks::basic_sink_frontend< char > > p;

  #if !defined(BOOST_LOG_NO_THREADS)
  // Asynchronous
  bool async = false;
  it = params.find("Asynchronous");
  if (it != params.end())
  {
    isstream strm(boost::any_cast<std::string>(it->second));
    strm.setf(std::ios_base::boolalpha);
    strm >> async;
  }

  // Construct the frontend, considering Asynchronous parameter
  if (!async)
    p = boost::make_shared< sinks::synchronous_sink< backend_t > >(backend);
  else
    p = boost::make_shared< sinks::asynchronous_sink< backend_t > >(backend);
#else
  // When multithreading is disabled we always use the unlocked sink frontend
  p = log::aux::new_shared< sinks::unlocked_sink< backend_t > >(backend);
#endif

  p->set_filter(filt);

  return p;
}


template <typename Settings, typename BackendPtr>
boost::shared_ptr<sinks::basic_sink_frontend<char> >
make_frontend(Settings const& params, BackendPtr& backend)
{
    typedef typename BackendPtr::element_type Backend;

    boost::shared_ptr<sinks::basic_sink_frontend<char> > forntend;

#if !defined(BOOST_LOG_NO_THREADS)
    // Asynchronous
    bool async = false;
    typedef std::map< std::string, boost::any > params_t;
    params_t::const_iterator it = params.find("Asynchronous");
    if (it != params.end())
    {
        std::string str = boost::any_cast<std::string>(it->second);
        std::basic_istringstream<char> strm(str);
        strm.setf(std::ios_base::boolalpha);
        strm >> async;
    }

    // Construct the frontend, considering Asynchronous parameter
    if (!async)
        forntend = boost::make_shared < sinks::synchronous_sink<Backend> > (backend);
    else
        forntend = boost::make_shared < sinks::asynchronous_sink<Backend> > (backend);
#else
    // When multithreading is disabled we always use the unlocked sink frontend
    forntend = boost::make_shared< sinks::unlocked_sink< Backend > >(backend);
#endif

    return forntend;
}

sinks::syslog::facility extract_facility(std::map< std::string, boost::any > const& params)
{
    typedef std::map< std::string, boost::any > params_t;
    params_t::const_iterator it = params.find("Facility");
    if (it == params.end())
        throw std::runtime_error("Facility parameter not specified for the syslog_native backend");

    std::string facility = boost::any_cast<std::string>(it->second);
    for(size_t i=0; ; i++)
    {
        if(!facilitynames[i].c_name)
            throw std::runtime_error("Facility parameter contains not proper value in syslog_native backend");

        if(facility == std::string(facilitynames[i].c_name))
            return sinks::syslog::make_facility(facilitynames[i].c_val);
    }

    return sinks::syslog::local7; // hide warnings
}


boost::shared_ptr< sinks::sink< char > >
create_syslog_native_sink(std::map< std::string, boost::any > const& params)
{
    sinks::syslog::facility facility = extract_facility(params);

    boost::shared_ptr< sinks::syslog_backend > backend =
        boost::make_shared< sinks::syslog_backend >(
                keywords::facility = facility,
                keywords::use_impl = sinks::syslog::native );

    typedef std::map< std::string, boost::any > params_t;
    params_t::const_iterator it = params.find("Format");
    if (it != params.end())
    {
        std::string str = boost::any_cast<std::string>(it->second);
        backend->set_formatter(logging::parse_formatter(str));
    }

    backend->set_severity_mapper(sinks::syslog::direct_severity_mapping<severity_level>("Severity"));

    boost::shared_ptr<sinks::basic_sink_frontend<char> > sink = make_frontend(params, backend);

    it = params.find("Filter");
    if (it != params.end())
    {
        std::string str = boost::any_cast<std::string>(it->second);
        sink->set_filter(logging::parse_filter(str));
    }

    return sink;
}

/*
 * emerg(0) > debug(7)
 * It's important for syslog
 */
class severity_filter_factory :
    public logging::basic_filter_factory<char, severity_level>
{
public:

    filter_type on_less_relation(string_type const& name, string_type const& arg)
    {
        return filters::attr< severity_level >(name) > boost::lexical_cast< severity_level >(arg);
    }

    filter_type on_greater_relation(string_type const& name, string_type const& arg)
    {
        return filters::attr< severity_level >(name) < boost::lexical_cast< severity_level >(arg);
    }

    filter_type on_less_or_equal_relation(string_type const& name, string_type const& arg)
    {
        return filters::attr< severity_level >(name) >= boost::lexical_cast< severity_level >(arg);
    }

    filter_type on_greater_or_equal_relation(string_type const& name, string_type const& arg)
    {
        return filters::attr< severity_level >(name) <= boost::lexical_cast< severity_level >(arg);
    }
};

std::ostream& operator<<(std::ostream& os, severity_level const& lvl)
{
    if (static_cast<std::size_t>(lvl) < levels_size)
        os << levels_formatted[lvl];
    else
        os << '#' << static_cast<unsigned int>(lvl) << '#';

    return os;
}

std::istream& operator>>(std::istream& is, severity_level& lvl)
{
    std::string str;
    is >> str;

    for (std::size_t i = 0; i < levels_size; ++i)
    {
        if (str == levels[i])
        {
            lvl = static_cast<severity_level>(i);
            return is;
        }
    }
    throw std::runtime_error(std::string("bad severity level: ") + str);
    return is; // hide compiler warnings
}

logging::formatter_types< char >::formatter_type
thread_id_formatter_factory(
    std::string const& attr_name,
    logging::formatter_types< char >::formatter_factory_args const& /*formatter_args*/)
{
  // Find the "format" argument
  return fmt::attr< boost::thread::id > (attr_name);
}

logging::formatter_types< char >::formatter_type
severity_formatter_factory(
    std::string const& attr_name,
    logging::formatter_types< char >::formatter_factory_args const& /*formatter_args*/)
{
  return fmt::attr< severity_level >(attr_name);
}

logging::formatter_types< char >::formatter_type
lineid_formatter_factory(
    std::string const& attr_name,
    logging::formatter_types< char >::formatter_factory_args const& formatter_args)
{
  typedef logging::formatter_types< char >::formatter_factory_args args;
  args::const_iterator it = formatter_args.find("format");
  if (it != formatter_args.end())
  {
    std::string fmt (it->second);
    if (fmt.size () >= 2 && fmt[0] == '"' && fmt[fmt.size ()-1] == '"')
    {
      fmt.erase (fmt.size ()-1);
      fmt.erase (0, 1);
    }
    return fmt::attr< unsigned int >(attr_name, fmt);
  }
  else
  {
    return fmt::attr< unsigned int >(attr_name);
  }
}

// Attribute interface class
class pid: public attrs::constant<pid_t>
{
    typedef attrs::constant<pid_t> base_type;
public:
    pid(): base_type(getpid())
    {
    }
    // Attribute casting support
    explicit pid(attrs::cast_source const& source): base_type(source)
    {
    }
};

void log_init (const boost::property_tree::ptree& cfg)
{
  logging::register_formatter_factory("ThreadID", &thread_id_formatter_factory);
  logging::register_formatter_factory("Severity", &severity_formatter_factory);
  logging::register_formatter_factory("LineID", &lineid_formatter_factory);

  boost::shared_ptr<severity_filter_factory> sp (new severity_filter_factory);
  logging::register_filter_factory<char> ("Severity", sp);
  {
    boost::log::aux::light_function1<
      boost::shared_ptr< sinks::sink< char > >,
      std::map< std::string, boost::any > const&
    > func1 (boost::bind (&create_multifile_sink, _1));

    boost::log::aux::light_function1<
      boost::shared_ptr< sinks::sink< char > >,
      std::map< std::string, boost::any > const&
    > func2 (boost::bind (&create_rotate_text_file_sink, _1));

    boost::log::aux::light_function1<
      boost::shared_ptr< sinks::sink< char > >,
      std::map< std::string, boost::any > const&
    > func3 (boost::bind (&create_syslog_native_sink, _1));

    logging::register_sink_factory("MultiFile", func1);
    logging::register_sink_factory("RotateTextFile", func2);
    logging::register_sink_factory("SyslogNative", func3);
  }
  log_load_cfg(cfg);
  boost::shared_ptr< logging::core > pCore = logging::core::get();
  pCore->add_global_attribute("Scope", attrs::named_scope());
  pCore->add_global_attribute("Tag", attrs::named_scope());
  pCore->add_global_attribute("PID", pid());

  logging::add_common_attributes();
}

void log_load_cfg(const boost::property_tree::ptree& cfg)
{
  using boost::property_tree::ptree;
  std::stringstream stream;
  boost::optional<const ptree&> options = cfg.get_child_optional("options");
  if (!options)
    return;
  for (ptree::const_iterator i_header = options->begin(),
       i_header_end = options->end(); i_header != i_header_end; ++i_header)
  {
    if (i_header->first == "<xmlcomment>")
      continue;

    stream << "[" << i_header->first << "]\n";
    for (ptree::const_iterator i_val = i_header->second.begin(),
         i_val_end = i_header->second.end(); i_val != i_val_end; ++i_val)
    {
        if (i_val->first != "<xmlcomment>")
            stream << i_val->first << "=" << i_val->second.get_value("") << "\n";
    }
  }
  logging::init_from_stream(stream);
}

YAMAIL_FQNS_LOG_END

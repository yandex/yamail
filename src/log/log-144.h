#include <yamail/log/log.h>
#include <yamail/log/rotate_text_file_backend.h>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <boost/log/core.hpp>
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
#include <boost/log/utility/scoped_attribute.hpp>
#include <boost/log/utility/init/from_stream.hpp>

#include <boost/shared_ptr.hpp>

YAMAIL_FQNS_LOG_BEGIN

namespace logging = boost::log;
namespace fmt = boost::log::formatters;
namespace flt = boost::log::filters;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;

static const char* levels[] =
{
  "debug", "info", "notice", "warn", "error", "fatal", "alert", "emerg"
};

boost::shared_ptr< sinks::sink< char > >
create_multifile_sink(std::map< std::string, std::string > const& params)
{
    // Read parameters for the backend and create it
    std::map< std::string, std::string >::const_iterator dir_par = params.find("Directory");
    std::map< std::string, std::string >::const_iterator attr_par = params.find("AttrName");
    if (dir_par == params.end())
        throw std::runtime_error("Directory parameter not specified for the MultiFile backend");
    if (attr_par == params.end())
        throw std::runtime_error("AttrName parameter not specified for the MultiFile backend");

    boost::shared_ptr< sinks::text_multifile_backend > backend =
        boost::make_shared< sinks::text_multifile_backend >();

    backend->set_file_name_composer(fmt::stream << fmt::fmt_wrapper< char, std::string >(dir_par->second)
                                                << fmt::attr< std::string >(attr_par->second)
                                                << ".log");

    // Construct and initialize the final sink
    boost::shared_ptr< sinks::synchronous_sink< sinks::text_multifile_backend > > sink =
        boost::make_shared< sinks::synchronous_sink< sinks::text_multifile_backend > >(backend);

    std::map< std::string, std::string >::const_iterator it = params.find("Filter");
    if (it != params.end())
        sink->set_filter(logging::parse_filter(it->second));

    return sink;
}

boost::shared_ptr< sinks::sink< char > >
create_rotate_text_file_sink(std::map< std::string, std::string > const& params)
{
  typedef std::basic_istringstream< char > isstream;
  typedef rotate_basic_text_file_backend< char > backend_t;
  typedef backend_t::path_type path_type;
  typedef std::map< std::string, std::string > params_t;
  boost::shared_ptr< backend_t > backend = boost::make_shared< backend_t >();

  // FileName
  params_t::const_iterator it =
      params.find("FileName");
  if (it != params.end())
    backend->set_file_name(it->second);
  else
    boost::log::aux::throw_exception(std::runtime_error("File name is not specified"));

  // Auto flush
  it = params.find("AutoFlush");
  if (it != params.end())
  {
    bool f = false;
    isstream strm(it->second);
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
    filt = logging::parse_filter(it->second);
  }

  // Formatter
  it = params.find("Format");
  if (it != params.end())
  {
    backend->set_formatter(logging::parse_formatter(it->second));
  }

  boost::shared_ptr< sinks::basic_sink_frontend< char > > p;

  #if !defined(BOOST_LOG_NO_THREADS)
  // Asynchronous
  bool async = false;
  it = params.find("Asynchronous");
  if (it != params.end())
  {
    isstream strm(it->second);
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

static const std::size_t levels_size = sizeof (levels) / sizeof (*levels);

std::ostream&
operator<< (std::ostream& os, severity_level const& lvl)
{
  if (static_cast<std::size_t>(max_sev_level) <= levels_size)
  {
    os << levels[lvl];
    for (std::size_t l = ::strlen (levels[lvl]); l < 6; ++l)
      os << ' ';
  }
  else
    os << '#' << lvl << '#';

  return os;
}

std::istream&
operator>> (std::istream& is, severity_level& lvl)
{
  std::string str;
  is >> str;

  for (std::size_t i = 0; i < levels_size; ++i)
  {
    if (str == levels[i])
    {
      lvl = static_cast<severity_level> (i);
      return is;
    }
  }

  throw std::runtime_error (std::string ("bad severity level: ") + str);
}

logging::formatter_types< char >::formatter_type
thread_id_formatter_factory(
    std::string const& attr_name,
    logging::formatter_types< char >::formatter_factory_args const& formatter_args)
{
  // Find the "format" argument
  typedef logging::formatter_types< char >::formatter_factory_args args;
  args::const_iterator it = formatter_args.find("format");
  return fmt::attr< boost::thread::id > (attr_name);
}

logging::formatter_types< char >::formatter_type
severity_formatter_factory(
    std::string const& attr_name,
    logging::formatter_types< char >::formatter_factory_args const& formatter_args)
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

void log_init (const boost::property_tree::ptree& cfg)
{
  logging::register_formatter_factory("ThreadID", &thread_id_formatter_factory);
  logging::register_formatter_factory("Severity", &severity_formatter_factory);
  logging::register_formatter_factory("LineID", &lineid_formatter_factory);

  typedef logging::basic_filter_factory<char, severity_level> severity_filter_factory;

  boost::shared_ptr<severity_filter_factory> sp (new severity_filter_factory);
  logging::register_filter_factory<char> ("Severity", sp);
  {
    boost::function1<
      boost::shared_ptr< sinks::sink< char > >,
      std::map< std::string, std::string > const&
    > func;
    func = create_multifile_sink;
    logging::register_sink_factory("MultiFile", func);
    func = create_rotate_text_file_sink;
    logging::register_sink_factory("RotateTextFile", func);
  }
  log_load_cfg(cfg);
  boost::shared_ptr< logging::core > pCore = logging::core::get();
  pCore->add_global_attribute("Scope", boost::make_shared< attrs::named_scope >());
  pCore->add_global_attribute("Tag", boost::make_shared< attrs::named_scope >());

  logging::add_common_attributes();
}

void log_load_cfg(const boost::property_tree::ptree& cfg)
{
  std::stringstream stream;
  boost::optional<const boost::property_tree::ptree&> options = cfg.get_child_optional("options");
  if (!options)
    return;
  for (boost::property_tree::ptree::const_iterator i_header = options->begin(),
       i_header_end = options->end(); i_header != i_header_end; ++i_header)
  {
    stream << "[" << i_header->first << "]\n";
    for (boost::property_tree::ptree::const_iterator i_val = i_header->second.begin(),
         i_val_end = i_header->second.end(); i_val != i_val_end; ++i_val)
    {
      stream << i_val->first << "=" << i_val->second.get_value("") << "\n";
    }
  }
  logging::init_from_stream(stream);
}

YAMAIL_FQNS_LOG_BEGIN


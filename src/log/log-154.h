#include <yamail/log/log.h>

#include <fstream>

#include <yamail/log/rotate_text_file_backend.h>
#include <yamail/compat/shared_ptr.h>

#include <boost/thread.hpp>
#include <boost/static_assert.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/filter_parser.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/log/utility/setup/from_stream.hpp>
#include <boost/log/utility/setup/from_settings.hpp>

YAMAIL_FQNS_LOG_BEGIN

using namespace YAMAIL_NS_COMPAT;
namespace logging = boost::log;
namespace fmt = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

//BOOST_LOG_GLOBAL_LOGGER_DEFAULT(global_logger, logger_t)
BOOST_LOG_GLOBAL_LOGGER_CTOR_ARGS(global_logger, logger_t, (keywords::channel = "general"))

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

template <typename Settings, typename BackendPtr>
shared_ptr<sinks::basic_formatting_sink_frontend<char> >
make_frontend(Settings const& settings, BackendPtr& backend)
{
    typedef typename BackendPtr::element_type Backend;

    shared_ptr<sinks::basic_formatting_sink_frontend<char> > forntend;

#if !defined(BOOST_LOG_NO_THREADS)
    // Asynchronous
    bool async = false;
    if (boost::optional<std::string> param = settings["Asynchronous"])
    {
        std::basic_istringstream<char> strm(param.get());
        strm.setf(std::ios_base::boolalpha);
        strm >> async;
    }

    // Construct the frontend, considering Asynchronous parameter
    if (!async)
        forntend = make_shared < sinks::synchronous_sink<Backend> > (backend);
    else
        forntend = make_shared < sinks::asynchronous_sink<Backend> > (backend);
#else
    // When multithreading is disabled we always use the unlocked sink frontend
    forntend = make_shared< sinks::unlocked_sink< Backend > >(backend);
#endif

    return forntend;
}


class multifile_sink_factory : public logging::sink_factory< char >
{
public:
    shared_ptr< sinks::sink >
    create_sink(settings_section const& settings)
    {
        // Read parameters for the backend and create it
        boost::optional< std::string > dir = settings["Directory"];
        boost::optional< std::string > attr = settings["AttrName"];
        if (!dir)
            throw std::runtime_error("Directory parameter not specified for the MultiFile backend");
        if (!attr)
            throw std::runtime_error("AttrName parameter not specified for the MultiFile backend");

        shared_ptr< sinks::text_multifile_backend > backend =
            make_shared< sinks::text_multifile_backend >();

        std::string dir_path = dir.get();
        backend->set_file_name_composer
            (
                sinks::file::as_file_name_composer
                (
                        expr::stream << dir.get() << fmt::attr< std::string >(attr.get()) << ".log"
                )
            );

        // Construct and initialize the final sink
        shared_ptr< sinks::synchronous_sink< sinks::text_multifile_backend > > sink =
            make_shared< sinks::synchronous_sink< sinks::text_multifile_backend > >(backend);

        boost::optional< std::string > filter = settings["Filter"];
        if (filter)
            sink->set_filter(logging::parse_filter(filter.get()));

        return sink;
    }
};

class rotate_text_file_sink_factory: public logging::sink_factory<char>
{
    shared_ptr<sinks::sink> create_sink(settings_section const& settings)
    {
        typedef std::basic_istringstream<char> isstream;
        typedef rotate_basic_text_file_backend<char> backend_t;
        shared_ptr<backend_t> backend = make_shared<backend_t>();

        // FileName
        if (boost::optional<std::string> param = settings["FileName"])
            backend->set_file_name(param.get());
        else
            boost::throw_exception(
                    std::runtime_error("File name is not specified"));

        // Auto flush
        if (boost::optional<std::string> param = settings["AutoFlush"])
        {
            bool f = false;
            isstream strm(param.get());
            strm.setf(std::ios_base::boolalpha);
            strm >> f;
            backend->auto_flush(f);
        }

        // Filter
        logging::filter filt;
        if (boost::optional<std::string> param = settings["Filter"])
        {
            filt = logging::parse_filter(param.get());
        }

        shared_ptr < sinks::basic_formatting_sink_frontend<char> > p;

#if !defined(BOOST_LOG_NO_THREADS)
        // Asynchronous
        bool async = false;
        if (boost::optional<std::string> param = settings["Asynchronous"])
        {
            isstream strm(param.get());
            strm.setf(std::ios_base::boolalpha);
            strm >> async;
        }

        // Construct the frontend, considering Asynchronous parameter
        if (!async)
            p = make_shared < sinks::synchronous_sink<backend_t>
                    > (backend);
        else
            p = make_shared < sinks::asynchronous_sink<backend_t>
                    > (backend);
#else
        // When multithreading is disabled we always use the unlocked sink frontend
        p = make_shared< sinks::unlocked_sink< backend_t > >(backend);
#endif

        p->set_filter(filt);

        // Formatter
        if (boost::optional<std::string> param = settings["Format"])
        {
            p->set_formatter(logging::parse_formatter(param.get()));
        }

        return p;
    }
};

class syslog_native_sink_factory : public logging::sink_factory< char >
{
public:
    shared_ptr< sinks::sink >
    create_sink(settings_section const& settings)
    {
        sinks::syslog::facility facility = extract_facility(settings);

        shared_ptr< sinks::syslog_backend > backend =
            make_shared< sinks::syslog_backend >(
                    keywords::facility = facility,
                    keywords::use_impl = sinks::syslog::native );

        backend->set_severity_mapper(sinks::syslog::direct_severity_mapping<severity_level>("Severity"));

        shared_ptr<sinks::basic_formatting_sink_frontend<char> > sink = make_frontend(settings, backend);

        if (boost::optional< std::string > param = settings["Filter"])
            sink->set_filter(logging::parse_filter(param.get()));

        if (boost::optional< std::string > param = settings["Format"])
            sink->set_formatter(logging::parse_formatter(param.get()));

        return sink;
    }

private:
    sinks::syslog::facility extract_facility(settings_section const& settings)
    {
        boost::optional<std::string> param = settings["Facility"];
        if(!param)
            throw std::runtime_error("Facility parameter not specified for the syslog_native backend");

        if(param.get().find_first_not_of("0123456789") != std::string::npos)
            throw std::runtime_error("Facility parameter contains not only digits in syslog_native backend");

        size_t facility = boost::lexical_cast<size_t>(param.get());
        if(facility > 23)
            throw std::runtime_error("Facility parameter out of ragne [0; 23] in syslog_native backend");

        return static_cast<sinks::syslog::facility>(facility*8);
    }
};

/*
 * emerg(0) > debug(7)
 * It's important for syslog
 */
class severity_filter_factory :
    public logging::basic_filter_factory<char, severity_level>
{
public:

    logging::filter on_less_relation(logging::attribute_name const& name, string_type const& arg)
    {
        return expr::attr< severity_level >(name) > boost::lexical_cast< severity_level >(arg);
    }

    logging::filter on_greater_relation(logging::attribute_name const& name, string_type const& arg)
    {
        return expr::attr< severity_level >(name) < boost::lexical_cast< severity_level >(arg);
    }

    logging::filter on_less_or_equal_relation(logging::attribute_name const& name, string_type const& arg)
    {
        return expr::attr< severity_level >(name) >= boost::lexical_cast< severity_level >(arg);
    }

    logging::filter on_greater_or_equal_relation(logging::attribute_name const& name, string_type const& arg)
    {
        return expr::attr< severity_level >(name) <= boost::lexical_cast< severity_level >(arg);
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

void log_init(const boost::property_tree::ptree& cfg)
{
    typedef logging::basic_formatter_factory<char, severity_level> severity_formatter_factory;
    //typedef logging::basic_filter_factory<char, severity_level> severity_filter_factory;

    logging::register_formatter_factory("Severity",
            make_shared<severity_formatter_factory>());
    logging::register_filter_factory("Severity",
            make_shared<severity_filter_factory>());

    logging::register_sink_factory("MultiFile",
            make_shared<multifile_sink_factory>());
    logging::register_sink_factory("RotateTextFile",
            make_shared<rotate_text_file_sink_factory>());
    logging::register_sink_factory("SyslogNative",
            make_shared<syslog_native_sink_factory>());

    log_load_cfg(cfg);
    shared_ptr < logging::core > pCore = logging::core::get();
    pCore->add_global_attribute("Scope", attrs::named_scope());
    pCore->add_global_attribute("Tag", attrs::named_scope());

    logging::add_common_attributes();
}

void log_load_cfg(const boost::property_tree::ptree& cfg)
{
    using boost::property_tree::ptree;

    std::stringstream stream;
    boost::optional<const ptree&> options = cfg.get_child_optional("options");
    if (!options)
        return;
    for (ptree::const_iterator i_header = options->begin(), i_header_end =
            options->end(); i_header != i_header_end; ++i_header)
    {
        if (i_header->first == "<xmlcomment>")
            continue;

        std::string sink = i_header->first;
        // For compatibility with old Boost.Log, we replace the "Sink:" prefix with "Sinks."
        // so that all sink parameters are placed in the common Sinks section.
        if (sink.compare(0, 5, "Sink:") == 0)
            sink = "Sinks." + sink.substr(5);

        stream << "[" << sink << "]\n";
        for (ptree::const_iterator i_val = i_header->second.begin(), i_val_end =
                i_header->second.end(); i_val != i_val_end; ++i_val)
        {
            if (i_val->first != "<xmlcomment>")
                stream << i_val->first << "=" << i_val->second.get_value("") << "\n";
        }
    }
    logging::init_from_stream(stream);
}

YAMAIL_FQNS_LOG_END


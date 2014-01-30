#ifndef _YAMAIL_LOG_LOG_H_
#define _YAMAIL_LOG_LOG_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>
#include <yamail/compat/shared_ptr.h>

#include <boost/version.hpp>
#include <boost/property_tree/ptree.hpp>

#ifndef YPLATFORM_WITHOUT_BOOST_LOG
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <yamail/compat/shared_ptr.h>
#else
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <iostream>
#endif

YAMAIL_FQNS_LOG_BEGIN

void log_init (const boost::property_tree::ptree& cfg);
void log_load_cfg(const boost::property_tree::ptree& cfg);

enum severity_level
{
  debug, info, notice, warning, error, fatal, alert, emerg, max_sev_level
};

#ifndef YPLATFORM_WITHOUT_BOOST_LOG

typedef boost::log::sources::severity_logger_mt<severity_level> source;

#if BOOST_VERSION < 104600
 BOOST_LOG_DECLARE_GLOBAL_LOGGER(global_logger, source)
#else
 BOOST_LOG_GLOBAL_LOGGER(global_logger, source)
#endif

#define YAMAIL_LOG_SEV(logger,severity) BOOST_LOG_SEV(logger, YAMAIL_FQNS_LOG::severity)
#define YAMAIL_GLOG_SEV(severity) \
            BOOST_LOG_SEV(YAMAIL_FQNS_LOG::global_logger::get(), YAMAIL_FQNS_LOG::severity)

#else
extern API_PUBLIC boost::mutex yamail_logger_mutex;

class yamail_logger
{
public:
     yamail_logger ()
    : lock_ (new boost::unique_lock<boost::mutex> (yamail_logger_mutex))
    {
        std::cout << time (0) << " ";
    }
    ~yamail_logger ()
    {
        std::cout << std::endl;
    }

    template <typename T>
    yamail_logger& operator<< (T t)
    {
        std::cout << t; return *this;
    }
private:
    YAMAIL_NS_COMPAT::shared_ptr< boost::unique_lock<boost::mutex> > lock_;
};

namespace detail
{
    template <class Context>
    inline std::string get_prefix(const Context& context)
    {
        if (context)
            return std::string(" [") + context->uniq_id() + std::string("] ");

        return std::string();
    }
} // namespace detail

#define L_(lvl) YAMAIL_FQNS_LOG::yplatform_logger ()
#define TASK_LOG(context, lvl) \
    YAMAIL_FQNS_LOG::yplatform_logger() << YAMAIL_FQNS_LOG::detail::get_prefix(context)

#endif // YPLATFORM_WITHOUT_BOOST_LOG

YAMAIL_FQNS_LOG_END

#endif // _YAMAIL_LOG_LOG_H_

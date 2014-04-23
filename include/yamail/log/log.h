#ifndef _YAMAIL_LOG_LOG_H_
#define _YAMAIL_LOG_LOG_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>

#include <boost/version.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>

YAMAIL_FQNS_LOG_BEGIN

void log_init (const boost::property_tree::ptree& cfg);
void log_load_cfg(const boost::property_tree::ptree& cfg);

enum severity_level
{
    emerg, alert, fatal, error, warning, notice, info, debug, end_of_sev_level
};

typedef boost::log::sources::severity_channel_logger_mt<severity_level> logger_t;

#if BOOST_VERSION < 104600
 BOOST_LOG_DECLARE_GLOBAL_LOGGER(global_logger, logger_t)
#else
 BOOST_LOG_GLOBAL_LOGGER(global_logger, logger_t)
#endif

#define YAMAIL_GLOBAL_LOG() YAMAIL_FQNS_LOG::global_logger::get()

#define YAMAIL_LOG_SEV(logger,severity) BOOST_LOG_SEV(logger, YAMAIL_FQNS_LOG::severity)
#define YAMAIL_GLOG_SEV(severity) \
    BOOST_LOG_SEV(YAMAIL_GLOBAL_LOG(), YAMAIL_FQNS_LOG::severity)


YAMAIL_FQNS_LOG_END

#endif // _YAMAIL_LOG_LOG_H_

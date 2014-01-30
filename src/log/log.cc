#include <boost/version.hpp>

#ifndef YPLATFORM_WITHOUT_BOOST_LOG

#if BOOST_VERSION < 104600
# include "log-144.h"
#elif BOOST_VERSION >= 104600 && BOOST_VERSION < 105400
# include "log-146.h"
#elif BOOST_VERSION >= 105400
# include "log-154.h"
#endif

#else

#include <yamail/log/log.h>
namespace yplatform { namespace log {

void log_init (const boost::property_tree::ptree&) {}
void log_load_cfg(const boost::property_tree::ptree&) {}
API_PUBLIC boost::mutex yamail_logger_mutex;

}}

#endif


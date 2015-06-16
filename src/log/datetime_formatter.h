#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>

YAMAIL_FQNS_LOG_BEGIN

class timestamp_formatter_factory:
  public boost::log::basic_formatter_factory<char, boost::posix_time::ptime>
{
public:
  formatter_type create_formatter(boost::log::attribute_name const& name,
      args_map const& args)
  {
    namespace expr = boost::log::expressions;
    typedef boost::posix_time::ptime ptime_type;
    args_map::const_iterator it = args.find("format");
    if (it != args.end())
      return expr::stream << expr::format_date_time<ptime_type>(
          expr::attr<ptime_type>(name), it->second);
    else
      return expr::stream << expr::attr<ptime_type>(name);
  }
};

YAMAIL_FQNS_LOG_END

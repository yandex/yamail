#ifndef _YAMAIL_LOG_TYPED_BACKEND_H_
#define _YAMAIL_LOG_TYPED_BACKEND_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>
#include <yamail/log/typed.h>
#include <yamail/log/typed_predefined.h>

#include <boost/phoenix.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/attributes/value_visitation.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
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
namespace attributes = boost::log::attributes;


#define TYPED_LOG(logger) YAMAIL_FQNS_LOG::typed::make_primary_stream (logger)
#define TYPED_TABLE_LOG(logger, table) \
  YAMAIL_FQNS_LOG::typed::make_primary_stream (logger, table)

template <typename Logger> class secondary_stream;

template <typename Logger>
class primary_stream
{
public:
	primary_stream (Logger& logger) : logger_ (logger) {}

#if 0
	primary_stream (Logger& logger, std::string const& table) 
	: logger_ (logger) 
	, sentry_ (new logging::scoped_attribute (logging::add_scoped_logger_attribute (logger_, "tskv_format",
	              attributes::make_constant(table))))
	{
	}
#endif

  Logger& logger () const { return logger_; }

private:
  Logger& logger_;
  // boost::scoped_ptr<logging::scoped_attribute> sentry_;

  template <typename L> 
  friend secondary_stream<L>
  operator<< (primary_stream<L> const& s, attributes_map& amap);

  // XXX: add && for c++11
  template <typename L>
  friend secondary_stream<L>
  operator<< (primary_stream<L> const& s, attr_type const& attr);
};

#if 1
  template <typename L, typename Helper>
  inline 
  typename boost::enable_if_c<detail::is_predefined<Helper>::value, secondary_stream<L> >::type
  operator<< (primary_stream<L> const& s, Helper const& helper)
  {
	  return secondary_stream<L> (s.logger (), helper ());
  }
#endif


template <typename Logger>
inline primary_stream<Logger>
make_primary_stream (Logger& logger)
{
	return primary_stream<Logger> (logger);
}

template <typename Logger>
inline primary_stream<Logger>
make_primary_stream (Logger& logger, std::string const& table)
{
	return primary_stream<Logger> (logger, table);
}

template <typename Logger>
class secondary_stream
{
public:
	secondary_stream (Logger& logger, attributes_map& amap) 
	  : logger_ (logger)
	  , amap_ptr_ (0)
	  , amap_ (amap)
	{}

	secondary_stream (Logger& logger, attr_type const& attr) 
	  : logger_ (logger)
	  , amap_ptr_ (new attributes_map)
	  , amap_ (*amap_ptr_)
	{
		try { amap_ << attr; }
		catch (...) { delete amap_ptr_; throw; }
	}

	~secondary_stream ()
	{

		// do actual logging...
#if 0
    BOOST_FOREACH (attr_name const& attr, cascade_keys (amap_))
    {
    	boost::optional<attr_value const&> val = cascade_find (amap_, attr);
    	if (val && ! is_deleted (*val))
      	  logger_ << attr << " = " << *val << "\n";
    }
#else
		BOOST_LOG(logger_) << logging::add_value ("tskv_attributes", amap_);
#endif
  }

private:
  Logger& logger_;
  attributes_map* amap_ptr_;
  attributes_map& amap_;

  // XXX: add && for c++11
  template <typename L> friend secondary_stream<L> const&
  operator<< (secondary_stream<L> const& s, attr_type const& attr)
  {
    s.amap_ << attr;
  	return s;
  }
};

template<typename Logger> struct predefined_traits<secondary_stream<Logger> >
{
	static const bool value = true;
};

template <typename Logger> inline secondary_stream<Logger> 
operator<< (primary_stream<Logger> const& s, attributes_map& amap)
{
	return secondary_stream<Logger> (s.logger_, amap);
}

template <typename Logger> inline secondary_stream<Logger> 
operator<< (primary_stream<Logger> const& s, attr_type const& attr)
{
	return secondary_stream<Logger> (s.logger_, attr);
}

} // namespace typed
#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_LOG_TYPED_BACKEND_H_

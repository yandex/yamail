/** @file */
#ifndef _YAMAIL_LOG_TYPED_BACKEND_H_
#define _YAMAIL_LOG_TYPED_BACKEND_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>
#include <yamail/log/typed.h>
#include <yamail/log/typed_predefined.h>

#include <yamail/compat/shared_ptr.h>

#include <boost/phoenix.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/attributes/value_visitation.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/attributes/constant.hpp>
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

/** @brief Log into logger */
#define TYPED_LOG(logger) YAMAIL_FQNS_LOG::typed::make_primary_stream (logger)

/**
 * Log typed record, specify the table.
 *
 * @param logger base logger.
 * @param table tskv table.
 */
#define TYPED_TABLE_LOG(logger, table) \
  YAMAIL_FQNS_LOG::typed::make_primary_stream (logger, table)

template <
    typename Logger
  , typename C
  , typename Tr = std::char_traits<C>
  , typename A = std::allocator<C>
>
class secondary_stream;

namespace detail {

#if YAMAIL_USE_RVALUES
template <typename T>
YAMAIL_FQNS_COMPAT::shared_ptr<T>
make_shared_ptr (T&& t)
{
	return YAMAIL_FQNS_COMPAT::make_shared<T> (std::forward<T> (t));
}
#else
template <typename T>
YAMAIL_FQNS_COMPAT::shared_ptr<T>
make_shared_ptr (T const& t)
{
	return YAMAIL_FQNS_COMPAT::make_shared<T> (t);
}
#endif

} // namespace detail

////////////////////////////////////////////////////////////////////////////////
template <typename Logger>
class primary_stream
{
public:
	primary_stream (Logger& logger) : logger_ (logger) {}

  template <typename C, typename Tr, typename A>
	primary_stream (Logger& logger, std::basic_string<C,Tr,A> const& table) 
	: logger_ (logger) 
	, sentry_ (
#if 0
	    detail::make_shared_ptr (
	      logging::add_scoped_logger_attribute (
	        logger_, "tskv_format", attributes::make_constant(table)
	      )
	    )
#else
			YAMAIL_FQNS_COMPAT::shared_ptr<
			    logging::aux::scoped_logger_attribute<Logger>
			> (
			  new logging::aux::scoped_logger_attribute< Logger >(
			    logger_, "tskv_format", attributes::make_constant(table))
			)
#endif
	  )
	{
	}

  Logger& logger () const { return logger_; }

private:
  Logger& logger_;
  YAMAIL_FQNS_COMPAT::shared_ptr<
      logging::aux::scoped_logger_attribute<Logger> > sentry_;

  template <typename L, typename C, typename Tr, typename A>
  friend secondary_stream<L,C,Tr,A>
  operator<< (primary_stream<L> const& s, basic_attributes_map<C,Tr,A>& amap);

  // XXX: add && for c++11
  template <typename L, typename C, typename Tr, typename A>
  friend secondary_stream<L,C,Tr,A>
  operator<< (primary_stream<L> const& s, 
      typename attr<C,Tr,A>::type const& attr);
};

////////////////////////////////////////////////////////////////////////////////
template<typename Logger, typename Predefined> 
inline typename boost::enable_if_c<
    detail::is_predefined<Predefined>::value
  , secondary_stream<Logger, typename Logger::char_type>
>::type
operator<< (
  primary_stream<Logger> const& strm, Predefined const& predefined)
{
	typedef typename Logger::char_type char_type;
	typedef std::char_traits<char_type> traits_type;
	typedef std::allocator<char_type> alloc_type;
  return secondary_stream<Logger, typename Logger::char_type> (
          strm.logger (), 
          predefined.template operator()<char_type,traits_type,alloc_type> ());
}

template <typename Logger>
inline primary_stream<Logger>
make_primary_stream (Logger& logger)
{
	return primary_stream<Logger> (logger);
}

template <typename Logger, typename C, typename Tr, typename A>
inline primary_stream<Logger>
make_primary_stream (Logger& logger, std::basic_string<C,Tr,A> const& table)
{
	return primary_stream<Logger> (logger, table);
}

template <typename Logger, typename CharT>
inline primary_stream<Logger>
make_primary_stream (Logger& logger, CharT const* table)
{
	return primary_stream<Logger> (logger, std::basic_string<CharT> (table));
}

////////////////////////////////////////////////////////////////////////////////
template <typename CharT> struct put_attr_map_in_logger;

template <> struct put_attr_map_in_logger<char>
{
	template <typename Logger, typename AttributesMap>
	static void apply (Logger& logger, AttributesMap& amap)
	{
	  BOOST_LOG(logger) << logging::add_value ("tskv_attributes", amap);
  }
};

template <> struct put_attr_map_in_logger<wchar_t>
{
	template <typename Logger, typename AttributesMap>
	static void apply (Logger& logger, AttributesMap& amap)
	{
	  BOOST_LOG(logger) << logging::add_value ("tskv_wattributes", amap);
  }
};

////////////////////////////////////////////////////////////////////////////////
template <typename Logger, typename C, typename Tr, typename A>
class secondary_stream
{
public:
  typedef basic_attributes_map<C,Tr,A> attributes_map_t;

public:
	secondary_stream (Logger& logger, attributes_map_t& amap) 
	  : logger_ (logger)
	  , amap_ptr_ (0)
	  , amap_ (amap)
	{}

	secondary_stream (Logger& logger, 
	    typename attr<C,Tr,A>::type const& attr) 
	  : logger_ (logger)
	  , amap_ptr_ (new attributes_map_t)
	  , amap_ (*amap_ptr_)
	{
		try { amap_ << attr; }
		catch (...) { delete amap_ptr_; throw; }
	}

	~secondary_stream () 
	{
		put_attr_map_in_logger<C>::apply (logger_, amap_);
  }

  attributes_map_t& amap () const { return amap_; }

private:
  Logger& logger_;
  attributes_map_t* amap_ptr_;
  attributes_map_t& amap_;

  // XXX: add && for c++11
  template <typename L, typename XC, typename XTr, typename XA>
  friend secondary_stream<L,XC,XTr,XA> const&
  operator<< (secondary_stream<L,XC,XTr,XA> const& s, 
      typename attr<C,Tr,A>::type const& attr)
  {
    s.amap_ << attr;
  	return s;
  }


  template <typename L, typename  XC, typename XTr, typename XA, typename P>
  friend typename boost::enable_if_c<
      detail::is_predefined<P>::value
   ,  secondary_stream<L,XC,XTr,XA> const&
  >::type
  operator<< (secondary_stream<L,XC,XTr,XA> const& s, P const& predefined)
  {
    s.amap () << predefined.template operator()<XC,XTr,XA> ();
    return s;
  }

};

////////////////////////////////////////////////////////////////////////////////
template <typename Logger, typename C, typename Tr, typename A> 
inline secondary_stream<Logger,C,Tr,A> 
operator<< (primary_stream<Logger> const& s, basic_attributes_map<C,Tr,A>& amap)
{
	return secondary_stream<Logger,C,Tr,A> (s.logger_, amap);
}

template <typename Logger, typename C, typename Tr, typename A> 
inline secondary_stream<Logger,C,Tr,A> 
operator<< (primary_stream<Logger> const& s, 
    typename attr<C,Tr,A>::type const& attr)
{
	return secondary_stream<Logger,C,Tr,A> (s.logger_, attr);
}

} // namespace typed
#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_LOG_TYPED_BACKEND_H_

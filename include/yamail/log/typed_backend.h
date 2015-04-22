/** @file */
#ifndef _YAMAIL_LOG_TYPED_BACKEND_H_
#define _YAMAIL_LOG_TYPED_BACKEND_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>
#include <yamail/log/typed.h>
#include <yamail/log/typed_predefined.h>
#if BOOST_VERSION <= 105300
#include <yamail/log/compat.h>
#endif

#include <yamail/compat/shared_ptr.h>

#include <boost/phoenix.hpp>

#include <boost/log/core.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/attributes/value_visitation.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/attributes/constant.hpp>

#if BOOST_VERSION > 105300
# include <boost/log/utility/manipulators/add_value.hpp>
#endif

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace log {
#else
YAMAIL_FQNS_LOG_BEGIN
#endif // GENERATING_DOCUMENTATION
namespace typed {

namespace logging = boost::log;
namespace src = boost::log::sources;
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
    typename C 
  , typename Tr = std::char_traits<C>
  , typename A = std::allocator<C>
>
class secondary_stream;

template <
    typename C 
  , typename Tr = std::char_traits<C>
  , typename A = std::allocator<C>
>
class primary_stream_base;

template <typename C, typename Tr, typename A>
secondary_stream<C,Tr,A>
operator<< (primary_stream_base<C,Tr,A> const& pstream, 
      basic_attributes_map<C,Tr,A> const& amap);

template <typename C, typename Tr, typename A>
secondary_stream<C,Tr,A>
operator<< (primary_stream_base<C,Tr,A> const& pstream, 
      typename attr<C,Tr,A>::type const& attr);

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
template <
    typename C
  , typename Tr
  , typename A
>
class primary_stream_base
{
public:
  typedef basic_attributes_map<C,Tr,A> attributes_map_t;

  // g++ does not let make it private.
  primary_stream_base (primary_stream_base const&);

	primary_stream_base () {}
  ~primary_stream_base () {}

  attributes_map_t& amap () const
  {
    return const_cast<attributes_map_t&> (amap_);
  }

private:
  attributes_map_t amap_;
  friend secondary_stream<C,Tr,A>
  operator<<<> (primary_stream_base<C,Tr,A> const& pstream, 
      attributes_map_t const& amap);

  // XXX: add && for c++11
  friend secondary_stream<C,Tr,A>
  operator<<<> (primary_stream_base<C,Tr,A> const& pstream, 
      typename attr<C,Tr,A>::type const& attr);
};

template <
    typename Logger
  , typename C = typename Logger::char_type
  , typename Tr = std::char_traits<C>
  , typename A = std::allocator<C>
>
class primary_stream : public primary_stream_base<C,Tr,A>
{
public:
	primary_stream (Logger& logger) : logger_ (logger), table_ (1, '-') {}

	primary_stream (Logger& logger, std::basic_string<C,Tr,A> const& table) 
    : logger_ (logger) 
    , table_ (table)
	{
	}

  Logger& logger () const { return logger_; }

  ~primary_stream ()
  {
#if BOOST_VERSION <= 105300
    typedef typename boost::log::aux::make_embedded_string_type<
             typename boost::remove_reference< std::basic_string<C,Tr,A> >::type
                  >::type embedded;

    typedef attributes::constant<embedded> constant_embedded;
    typedef attributes::constant<typename primary_stream::attributes_map_t>
        constant_attributes_map;

    BOOST_LOG_SCOPED_LOGGER_ATTR_CTOR (logger_, "tskv_format", 
        constant_embedded, (compat::make_constant(table_)));

    BOOST_LOG_SCOPED_LOGGER_ATTR_CTOR (logger_, "tskv_attributes", 
      constant_attributes_map, (compat::make_constant (this->amap ())));
#else
    BOOST_LOG_SCOPED_LOGGER_TAG (logger_, "tskv_format", table_);

    BOOST_LOG_SCOPED_LOGGER_TAG (logger_, "tskv_attributes", this->amap ());
#endif
    BOOST_LOG(logger_) << "";

  }

private:
  Logger& logger_;
  std::basic_string<C,Tr,A> table_;
};

////////////////////////////////////////////////////////////////////////////////
template<class C, class Tr, class A, typename Predefined> 
inline typename boost::enable_if_c<
    detail::is_predefined<Predefined>::value
  , secondary_stream<C,Tr,A>
>::type
operator<< (
  primary_stream_base<C,Tr,A> const& pstream, Predefined const& predefined)
{
  pstream.amap () << predefined.template operator()<C,Tr,A> ();
  return secondary_stream<C,Tr,A> (pstream);
}

////////////////////////////////////////////////////////////////////////////////
template <typename C, typename Tr, typename A>
class secondary_stream
{
  primary_stream_base<C,Tr,A> const& pstream_;


public:
  typedef basic_attributes_map<C,Tr,A> attributes_map_t;

  secondary_stream (primary_stream_base<C,Tr,A> const& ps) 
    : pstream_ (ps) {}

private:
  attributes_map_t& amap () const { return pstream_.amap (); }

  // XXX: add && for c++11
  friend secondary_stream<C,Tr,A> const&
  operator<< (secondary_stream<C,Tr,A> const& s, 
      typename attr<C,Tr,A>::type const& attr)
  {
    s.amap () << attr;
  	return s;
  }

  friend secondary_stream<C,Tr,A> const&
  operator<< (secondary_stream<C,Tr,A> const& s, 
      basic_attributes_map<C,Tr,A> const& amap)
  {
  	s.amap () = scoped (s.amap ());
#if YAMAIL_CPP < 11
    typedef typename attr<C,Tr,A>::name attr_name;
    BOOST_FOREACH (attr_name const& aname, cascade_keys (amap))
#else
    for (auto const& aname: cascade_keys (amap))
#endif
    {
      // TODO: optimize, use stored attributes instead of constructing it again here.
      if (boost::optional<typename attr<C,Tr,A>::value const&> aval =
           cascade_find (amap, aname))
        if (! is_deleted (*aval))
        {
          s << typename attr<C,Tr,A>::type (aname, *aval);
        }
    }

  	return s;
  }

  template <typename P>
  friend typename boost::enable_if_c<
      detail::is_predefined<P>::value
   ,  secondary_stream<C,Tr,A> const&
  >::type
  operator<< (secondary_stream<C,Tr,A> const& s, P const& predefined)
  {
    s.amap () << predefined.template operator()<C,Tr,A> ();
    return s;
  }

};

template <typename C, typename Tr, typename A>
secondary_stream<C,Tr,A>
operator<< (primary_stream_base<C,Tr,A> const& pstream, 
      basic_attributes_map<C,Tr,A> const& amap)
{
  pstream.amap () = amap;
  return secondary_stream<C,Tr,A> (pstream);
}

template <typename C, typename Tr, typename A>
secondary_stream<C,Tr,A>
operator<< (primary_stream_base<C,Tr,A> const& pstream, 
      typename attr<C,Tr,A>::type const& attr)
{
  pstream.amap () << attr;
  return secondary_stream<C,Tr,A> (pstream);
}

////////////////////////////////////////////////////////////////////////////////
template <typename LL>
inline primary_stream<LL>
make_primary_stream (LL& logger)
{
  return primary_stream<LL> (logger);
}

template <typename LL, typename XC, typename XTr, typename XA>
inline primary_stream<LL,XC,XTr,XA>
make_primary_stream (LL& logger, std::basic_string<XC,XTr,XA> const& table)
{
  return primary_stream<LL,XC,XTr,XA> (logger, table);
}

template <typename LL, typename XC>
inline primary_stream<LL,XC>
make_primary_stream (LL& logger, XC const* table)
{
  return primary_stream<LL,XC> (logger, std::basic_string<XC> (table));
}


} // namespace typed
#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_LOG_TYPED_BACKEND_H_

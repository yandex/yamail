#ifndef _YAMAIL_LOG_TYPED_BACKEND_H_
#define _YAMAIL_LOG_TYPED_BACKEND_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>
#include <yamail/log/typed.h>
#include <yamail/log/typed_predefined.h>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace log {
#else
YAMAIL_FQNS_LOG_BEGIN
#endif // GENERATING_DOCUMENTATION
namespace typed {

#define TYPED_LOG(logger) YAMAIL_FQNS_LOG::typed::make_primary_stream (logger)

template <typename Logger> class secondary_stream;

template <typename Logger>
class primary_stream
{
public:
	primary_stream (Logger& logger) : logger_ (logger) {}

private:
  Logger& logger_;

  template <typename L> 
  friend secondary_stream<L>
  operator<< (primary_stream<L> const& s, attributes_map& amap);

  // XXX: add && for c++11
  template <typename L>
  friend secondary_stream<L>
  operator<< (primary_stream<L> const& s, attr_type const& attr);
};

template <typename Logger>
inline primary_stream<Logger>
make_primary_stream (Logger& logger)
{
	return primary_stream<Logger> (logger);
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

	secondary_stream (Logger& logger, attr_type& attr) 
	  : logger_ (logger)
	  , amap_ptr_ (new attributes_map)
	  , amap_ (*amap_ptr_)
	{
	  std::unique_ptr<attributes_map> tmp (amap_ptr_);
	  amap_ << attr;
	  tmp.release ();
	}

	~secondary_stream ()
	{
		// do actual logging...
    BOOST_FOREACH (attr_name const& attr, cascade_keys (amap_))
    {
    	boost::optional<attr_value const&> val = cascade_find (amap_, attr);
    	if (val && ! is_deleted (*val))
      	  logger_ << attr << " = " << *val << "\n";
    }
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
	typedef secondary_stream<Logger> const& result_type;
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

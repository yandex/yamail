#ifndef _YAMAIL_LOG_TYPED_BACKEND_ATTR_H_
#define _YAMAIL_LOG_TYPED_BACKEND_ATTR_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>

#include <yamail/log/typed.h>

#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/attributes/attribute_value_impl.hpp>

#if 0
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

typedef attributes::attribute_value_impl<attributes_map> map_attribute_value;



} // namespace typed
#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION
#endif
#endif // _YAMAIL_LOG_TYPED_BACKEND_ATTR_H_

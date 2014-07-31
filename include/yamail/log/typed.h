#ifndef _YAMAIL_LOG_TYPED_H_
#define _YAMAIL_LOG_TYPED_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>

#include <string>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace log {
#else
YAMAIL_FQNS_LOG_BEGIN
#endif // GENERATING_DOCUMENTATION

/// Log field definition types enum.
enum type_t {
	TYPE_NUMBER,        ///< number type.
	TYPE_REAL,          ///< real (float or double) type.
  TYPE_STRING,        ///< string type.
  TYPE_IP,            ///< IPv4 or IPv6 type.
  TYPE_IP_ENDPOINT,   ///< IP + port.
  TYPE_TIME           ///< Time type (chrono or unixtime).
};

/// Rules definition enum.
enum rule_t {
	RULE_MANDATORY,     ///< Mandatory field.
	RULE_OPTIONAL,      ///< Optional field.
	RULE_GENERATED,     ///< Field generated with library.
	RULE_AUTO           ///< User supply or libaray generates.
};

/// Fields definition structure.
struct field_def 
{
	std::string name;   ///< Field name.
	type_t      type;   ///< Field type.
	rule_t      rule;   ///< Field rule.
	std::string descr;  ///< Field description.
};

/// Predefined field types.
enum well_known_field_enum {
	arg_time,           ///< Represents time of the log record.
	arg_service,        ///< Represents name of the service (name of the daemon).
	arg_pid,            ///< Represents ID of the logging process.
	arg_ppid,           ///< Represents Parent Process ID of logging process.
	arg_tid,            ///< Represents ID of logging thread.
	arg_priority,       ///< Represents logging record priority.
	
	_key_enum_last
};

_constexpr inline char const& 
well_known_field (well_known_field_enum f) _noexcept
{
	static char* fields[] = {
		"TIME", "SERVICE", "PID", "PPID", "TID", "PRIORITY"
	};

  
	return fields[ static_cast<int> (f) ];
}

typedef boost::type_erasure::any<
  boost::mpl::vector<
      boost::type_erasure::copy_constructible<>
    , boost::type_erasure::typeid<>
    , boost::type_erasure::ostreamable<>
    , boost::type_erasure::relaxed
  >
> field_value;

typedef boost::variant<well_known_field_enum, std::string> field_name;
typedef std::pair<field_name const, field_value> field_type;

struct deleted_t {};

namespace { deleted_t deleted; }

/// Creates field from string and value.
/**
 * @param key field name.
 * @param value field value.
 * @returns field definition instance.
 */
template <typename T>
field_type 
make_field (std::string const& key, T const& value)
{
	return field_type (key, value);
}

/// Creates field from well known key enum ID and value.
/**
 * @param key field enum ID.
 * @param value field value.
 * @returns field definition instance.
 */
template <typename T>
field_type 
make_field (well_known_field_enum key, T const& value)
{
	return field_type (key, value);
}

/// Typed log attributes map.
class typed_attributes_map 
{
public:
  /// Constructs insternal map from initiaizer_list.
  typed_attributes_map (
    std::initializer_list<field_type> fields, 
    typed_attributes_map* parent = 0)
  : parent_ (parent)
  {
  	map_.insert (std::move (fields));
  }

protected:
  typedef std::map<field_name const, any> map_type;

private:
  typed_attributes_map* parent_;
  map_type map_;

};


#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_LOG_TYPED_H_

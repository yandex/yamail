#ifndef _YAMAIL_LOG_TYPED_H_
#define _YAMAIL_LOG_TYPED_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>

#include <yamail/compat/shared_ptr.h>

#include <yamail/utility/list_of.h>

#include <boost/type_erasure/builtin.hpp>
#include <boost/type_erasure/operators.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/variant.hpp>

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <boost/foreach.hpp>

#include <string>
#include <map>
#include <utility> // std::pair

#include <iostream>

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

//_constexpr 
inline char const* 
well_known_field (well_known_field_enum f) _noexcept
{
	static _constexpr char const* fields[] = {
		"TIME", "SERVICE", "PID", "PPID", "TID", "PRIORITY"
	};

  
	return fields[ static_cast<int> (f) ];
}

typedef boost::type_erasure::any<
  boost::mpl::vector<
      boost::type_erasure::copy_constructible<>
    , boost::type_erasure::typeid_<>
    , boost::type_erasure::ostreamable<>
    , boost::type_erasure::relaxed
  >
> field_value;

typedef boost::variant<well_known_field_enum, std::string> field_name;
typedef std::pair<field_name const, field_value> field_type;

struct deleted_t {};

template <typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<< (std::basic_ostream<CharT, Traits>& os, deleted_t const&)
{
	return os << "(deleted)";
}

namespace { deleted_t deleted; }

typedef YAMAIL_FQNS_UTILITY::list_of<field_type> field_list;
typedef YAMAIL_FQNS_UTILITY::list_of<field_name> name_list;

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
protected:
  typedef std::map<field_name const, field_value> map_type;

  struct proxy;
  typedef YAMAIL_FQNS_COMPAT::shared_ptr<proxy> proxy_ptr;

  struct proxy 
  {
  	proxy (proxy_ptr parent = proxy_ptr ())
  	  : parent (parent)
  	{
    }

    proxy_ptr parent;
    map_type map;
  };

  map_type      & map ()       { return proxy_->map; }
  map_type const& map () const { return proxy_->map; }

  proxy_ptr      & parent ()       { return proxy_->parent; }
  proxy_ptr const& parent () const { return proxy_->parent; }

private:
  struct make_deleted
  {
  	template <class> struct result { typedef field_type type; };

  	field_type operator() (field_name const& name) const
  	{ 
  		return field_type ("", ""); 
  	}
  };
public:

  //////////////////////////////////////////////////////////////////////////////
  // Constructor.

#if YAMAIL_CPP >= 11
  /// Constructs insternal map from initiaizer_list.
  /**
   * @param fields fileds definition list.
   * @param parent parent typed_attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  typed_attributes_map (
    std::initializer_list<field_type> fields, 
    proxy_ptr parent = proxy_ptr ())
  : proxy_ (new proxy)
  {
  	proxy_->parent = parent;
  	proxy_->map.insert (fields);
  }
#endif // YAMAIL_CPP >= 11
  
  /// Constructs insternal map from field_list.
  /**
   * @param fields fileds definition list.
   * @param parent parent typed_attributes_map.
   */
  typed_attributes_map (field_list const& fields, 
      proxy_ptr parent = proxy_ptr ())
  : proxy_ (new proxy)
  {
    proxy_->parent = parent;
		BOOST_FOREACH (field_type const& field, fields)
		{
    	proxy_->map.insert (field);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Replace.

#if YAMAIL_CPP >= 11
  /// Replaces of adds given fields.
  /**
   * @param fields list of fields to replace or add.
   * @returns reference to this typed_attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  typed_attributes_map&
  replace (std::initializer_list<field_type> fields)
  {
  	for (auto const& field: fields)
    {
    	proxy_->map[std::get<0> (field)] = std::get<1> (field);
    }

    return *this;
  }
#endif // YAMAIL_CPP >= 11

  /// Replaces of adds given fields.
  /**
   * @param fields list of fields to replace or add.
   * @returns reference to this typed_attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  typed_attributes_map&
  replace (field_list const& fields)
  {
		BOOST_FOREACH (field_type const& field, fields)
    {
    	proxy_->map[std::get<0> (field)] = std::get<1> (field);
    }

    return *this;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Erase.

#if YAMAIL_CPP >= 11
  /// Removes given fields from internal map.
  /**
   * @param names list of the names.
   * @returns reference to this typed_attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  typed_attributes_map&
  erase (std::initializer_list<field_name> names)
  {
  	for (auto const& name: names)
    {
      proxy_->map.erase (name);
    }

    return *this;
  }
#endif // YAMAIL_CPP >= 11

  /// Removes given fields from internal map.
  /**
   * @param names list of the names.
   * @returns reference to this typed_attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  typed_attributes_map&
  erase (name_list const& names)
  {
		BOOST_FOREACH (field_name const& name, names)
    {
      proxy_->map.erase (name);
    }

    return *this;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Scoped Change.

#if YAMAIL_CPP >= 11
  /// Creates new scoped typed_attributes_map.
  /** 
   * @param to_add fields list to be added into scoped map.
   * @param to_del fields names to be deleted from scoped map.
   * @returns new scoped map instance.
   */
  typed_attributes_map
  scoped_change (std::initializer_list<field_type> to_add,
    std::initializer_list<field_name> to_del = 
        std::initializer_list<field_name> ()) const _noexcept
  {
  	typed_attributes_map tmp (to_add, proxy_);

  	boost::copy (
  	  to_del | boost::adaptors::transformed (
  	    [] (field_name name) { return field_type (std::move (name), deleted); }
  	  ),
  	  std::inserter (tmp.proxy_->map, tmp.proxy_->map.end ()));

  	return tmp;
  }
#endif // YAMAIL_CPP >= 11

  /// Creates new scoped typed_attributes_map.
  /** 
   * @param to_add fields list to be added into scoped map.
   * @param to_del fields names to be deleted from scoped map.
   * @returns new scoped map instance.
   */
  typed_attributes_map
  scoped_change (field_list const& to_add, 
      name_list to_del = name_list ()) const
  {
  	typed_attributes_map tmp (to_add, proxy_);

  	boost::copy (
  	  to_del | boost::adaptors::transformed (make_deleted ()),
  	  std::inserter (tmp.proxy_->map, tmp.proxy_->map.end ())
  	);
  	return tmp;
  }

private:
  proxy_ptr proxy_;
};


#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_LOG_TYPED_H_

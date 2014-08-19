#ifndef _YAMAIL_LOG_TYPED_H_
#define _YAMAIL_LOG_TYPED_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>

#include <yamail/compat/shared_ptr.h>

#include <yamail/utility/list_of.h>
#include <yamail/utility/update_iterator.h>

#include <boost/type_erasure/builtin.hpp>
#include <boost/type_erasure/operators.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/variant.hpp>

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <boost/foreach.hpp>

#include <syslog.h>

#include <string>
#include <map>
#include <set>
#include <utility> // std::pair
#include <algorithm> // std:move

#include <iostream>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace log {
#else
YAMAIL_FQNS_LOG_BEGIN
#endif // GENERATING_DOCUMENTATION
namespace typed {

/// Log attribute definition types enum.
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
	  RULE_MANDATORY      ///< Mandatory field.
	, RULE_OPTIONAL       ///< Optional field.
	, RULE_IGNORED        ///< Field ignored, type is not checked.
	, RULE_STRICT_IGNORED ///< Field ignored, but type is checked.
	, RULE_GENERATED      ///< Field generated with library.
	, RULE_AUTO           ///< User supply or libaray generates.
};

/// Fields definition structure.
struct attribute_def 
{
	std::string name;   ///< Field name.
	type_t      type;   ///< Field type.
	rule_t      rule;   ///< Field rule.
	std::string descr;  ///< Field description.
};

/// Predefined log priorities type (syslog compatible).
enum priority_enum {
	  log_emerg   = LOG_EMERG
	, log_alert   = LOG_ALERT
	, log_crit    = LOG_CRIT
	, log_err     = LOG_ERR
	, log_warning = LOG_WARNING
	, log_warn    = LOG_WARNING
	, log_notice  = LOG_NOTICE
	, log_info    = LOG_INFO
	, log_debug   = LOG_DEBUG
};

/// Predefined attribute types.
enum well_known_attr_enum {
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
well_known_attr (well_known_attr_enum f) _noexcept
{
	static _constexpr char const* attributes[] = {
		"TIME", "SERVICE", "PID", "PPID", "TID", "PRIORITY"
	};

  
	return attributes[ static_cast<int> (f) ];
}

typedef boost::type_erasure::any<
  boost::mpl::vector<
      boost::type_erasure::copy_constructible<>
    , boost::type_erasure::typeid_<>
    , boost::type_erasure::ostreamable<>
    , boost::type_erasure::relaxed
  >
> attr_value;

typedef boost::variant<well_known_attr_enum, std::string> attr_name;
typedef std::pair<attr_name const, attr_value> attr_type;

template <typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<< (std::basic_ostream<CharT, Traits>& os, attr_type const& ft)
{
	return os << '[' << ft.first << "=>" << ft.second << ']';
}

struct deleted_t {};

template <typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<< (std::basic_ostream<CharT, Traits>& os, deleted_t const&)
{
	return os << "(deleted)";
}

namespace { const deleted_t deleted = deleted_t (); }

namespace detail {
struct make_deleted
{
  template <class> struct result { typedef attr_type type; };

#if YAMAIL_CPP >= 11
  attr_type operator() (attr_name&& name) const
  { 
    return attr_type (std::move (name), deleted); 
  }
#endif

  attr_type operator() (attr_name const& name) const
  { 
    return attr_type (name, deleted); 
  }
};
} // namespace detail

typedef YAMAIL_FQNS_UTILITY::list_of<attr_type> attr_list;
typedef YAMAIL_FQNS_UTILITY::list_of<attr_name> name_list;

#if YAMAIL_CPP >= 11
/// Creates attr from string and value.
/**
 * @param key attr name.
 * @param value attr value.
 * @returns attr definition instance.
 * @note This method is only defined in C++11 and above compile mode.
 */
template <typename T>
attr_type 
make_attr (std::string&& key, T value)
{
	return attr_type (std::move (key), value);
}
#endif

/// Creates attr from string and value.
/**
 * @param key attr name.
 * @param value attr value.
 * @returns attr definition instance.
 */
template <typename T>
attr_type 
make_attr (std::string const& key, T const& value)
{
	return attr_type (key, value);
}

/// Creates attr from well known key enum ID and value.
/**
 * @param key attr enum ID.
 * @param value attr value.
 * @returns attr definition instance.
 */
template <typename T>
attr_type 
make_attr (well_known_attr_enum key, T const& value)
{
	return attr_type (key, value);
}

#if YAMAIL_CPP >= 11
/// Delete helper function.
/**
 * @param key attr name.
 * @returns 'deleter' attr definition instance.
 * @note This method is only defined in C++11 and above compile mode.
 */
template <typename T>
attr_type 
delete_attr (std::string&& key)
{
	return attr_type (std::move (key), deleted);
}
#endif

/// Delete helper function.
/**
 * @param key attr name.
 * @returns 'deleter' attr definition instance.
 * @note This method is only defined in C++11 and above compile mode.
 */
template <typename T>
attr_type 
delete_attr (std::string const& key)
{
	return attr_type (key, deleted);
}

/// $Delete helper function.
/**
 * @param key attr enum ID.
 * @returns 'deleter' attr definition instance.
 */
template <typename T>
attr_type 
delete_attr (well_known_attr_enum key)
{
	return attr_type (key, deleted);
}

/// Typed log attributes map.
class attributes_map 
{
protected:
  typedef std::map<attr_name const, attr_value> map_type;

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

public:

  //////////////////////////////////////////////////////////////////////////////
  // Constructors.

  /// Constructs attributes map based on given map.
  /**
   * @param parent parent attributes_map.
   */
  attributes_map (proxy_ptr parent = proxy_ptr ())
  : proxy_ (new proxy)
  {
    proxy_->parent = parent;
  }

#if YAMAIL_CPP >= 11
  /// Constructs attributes map from initiaizer_list.
  /**
   * @param attrs fileds definition list.
   * @param parent parent attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  attributes_map (
    std::initializer_list<attr_type> attrs, 
    proxy_ptr parent = proxy_ptr ())
  : proxy_ (new proxy)
  {
  	proxy_->parent = parent;
  	proxy_->map.insert (attrs);
  }
#endif // YAMAIL_CPP >= 11
  
  /// Constructs attributes map from attr_list.
  /**
   * @param attrs fileds definition list.
   * @param parent parent attributes_map.
   */
  attributes_map (attr_list const& attrs, 
      proxy_ptr parent = proxy_ptr ())
  : proxy_ (new proxy)
  {
    proxy_->parent = parent;
		BOOST_FOREACH (attr_type const& attr, attrs)
		{
    	proxy_->map.insert (attr);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Replace.

#if YAMAIL_CPP >= 11
  /// Replaces of adds given attr.
  /**
   * @param attr the attrs to replace or add.
   * @returns reference to this attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  inline attributes_map&
  replace (attr_type&& attr)
  {
  	proxy_->map[std::move (attr.first)] = std::move (attr.second);
    return *this;
  }
#endif

  /// Replaces of adds given attr.
  /**
   * @param attr the attrs to replace or add.
   * @returns reference to this attributes_map.
   */
  inline attributes_map&
  replace (attr_type const& attr)
  {
  	proxy_->map[attr.first] = attr.second;
    return *this;
  }

#if YAMAIL_CPP >= 11
  /// Replaces of adds given attrs.
  /**
   * @param attrs list of attrs to replace or add.
   * @returns reference to this attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  attributes_map&
  replace (std::initializer_list<attr_type> attrs)
  {
  	for (auto const& attr: attrs)
    {
    	replace (attr);
    }

    return *this;
  }
#endif // YAMAIL_CPP >= 11

  /// Replaces of adds given attrs.
  /**
   * @param attrs list of attrs to replace or add.
   * @returns reference to this attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  attributes_map&
  replace (attr_list const& attrs)
  {
		BOOST_FOREACH (attr_type const& attr, attrs)
    {
    	replace (attr);
    }

    return *this;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Erase.

  /// Erase given attr from internal map.
  /**
   * @param names attr name to be erased.
   * @returns reference to this attributes_map.
   */
  inline attributes_map&
  erase (attr_name const& name)
  {
    proxy_->map.erase (name);
    return *this;
  }

#if YAMAIL_CPP >= 11
  /// Removes given attrs from internal map.
  /**
   * @param names list of the names.
   * @returns reference to this attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  attributes_map&
  erase (std::initializer_list<attr_name> names)
  {
  	for (auto const& name: names)
    	erase (name);

    return *this;
  }
#endif // YAMAIL_CPP >= 11

  /// Removes given attrs from internal map.
  /**
   * @param names list of the names.
   * @returns reference to this attributes_map.
   */
  attributes_map&
  erase (name_list const& names)
  {
		BOOST_FOREACH (attr_name const& name, names)
    {
      erase (name);
    }

    return *this;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Scoped Change.

#if YAMAIL_CPP >= 11
  /// Creates new scoped attributes_map.
  /** 
   * @param to_add attrs list to be added into scoped map.
   * @param to_del attrs names to be deleted from scoped map.
   * @returns new scoped map instance.
   * @note This method is only defined in C++11 and above compile mode.
   */
  attributes_map
  scoped_change (std::initializer_list<attr_type> to_add,
    std::initializer_list<attr_name> to_del = 
        std::initializer_list<attr_name> ()) const 
  {
  	attributes_map tmp (to_add, proxy_);

  	boost::copy (
  	  to_del | boost::adaptors::transformed (
  	    [] (attr_name name) { return attr_type (std::move (name), deleted); }
  	  ),
  	  YAMAIL_FQNS_UTILITY::updater (tmp.proxy_->map)
  	);

  	return tmp;
  }
#endif // YAMAIL_CPP >= 11

  /// Creates new scoped attributes_map.
  /** 
   * @param to_add attrs list to be added into scoped map.
   * @param to_del attrs names to be deleted from scoped map.
   * @returns new scoped map instance.
   */
  attributes_map
  scoped_change (attr_list const& to_add, 
      name_list to_del = name_list ()) const
  {
  	attributes_map tmp (to_add, proxy_);

  	boost::copy (
  	  to_del | boost::adaptors::transformed (detail::make_deleted ()),
  	  YAMAIL_FQNS_UTILITY::updater (tmp.proxy_->map)
  	);
  	return tmp;
  }

protected:
  boost::optional<attr_value const&>
  cascade_find (attr_name const& name) const
  {
  	for (proxy_ptr proxy = proxy_; proxy; proxy = proxy->parent)
    {
  	  map_type::const_iterator found = proxy->map.find (name);
  	  if (found != proxy->map.end ()) return found->second;
    }

    return boost::optional<attr_value const&> ();
  }

  std::set<attr_name> 
  cascade_keys ()
  {
  	// TODO: possible optimization is to strip deleted entries from keys.
  	std::set<attr_name> keys;

    for (proxy_ptr proxy = proxy_; proxy; proxy = proxy->parent)
    {
    	boost::copy(proxy->map | boost::adaptors::map_keys,
    	    std::inserter (keys, keys.end ()));
    }

    return keys;
  }

 private:
  friend attributes_map scoped (attributes_map& map);

  proxy_ptr proxy_;
};

/// Create new scope from given map.
/**
 * @param map attributes map.
 * @returns new scope based on map.
 */
inline attributes_map
scoped (attributes_map& map)
{
	return attributes_map (map.proxy_);
}

/// Add or replace attr in given map.
/**
 * @param map attributes map.
 * @param attr attr to add or replace.
 * @returns Reference to map.
 */
inline attributes_map&
operator<< (attributes_map& map, attr_type const& attr)
{
  return map.replace (attr); 
}

} // namespace typed
#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_LOG_TYPED_H_

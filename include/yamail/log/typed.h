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

#include <string>
#include <map>
#include <utility> // std::pair

#include <iostream>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace log {
#else
YAMAIL_FQNS_LOG_BEGIN
#endif // GENERATING_DOCUMENTATION
namespace typed {

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
	  RULE_MANDATORY      ///< Mandatory field.
	, RULE_OPTIONAL       ///< Optional field.
	, RULE_IGNORED        ///< Field ignored, type is not checked.
	, RULE_STRICT_IGNORED ///< Field ignored, but type is checked.
	, RULE_GENERATED      ///< Field generated with library.
	, RULE_AUTO           ///< User supply or libaray generates.
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

template <typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<< (std::basic_ostream<CharT, Traits>& os, field_type const& ft)
{
	return os << '[' << std::get<0> (ft) 
	  << "=>" << std::get<1> (ft) << ']';
}

struct deleted_t {};

template <typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<< (std::basic_ostream<CharT, Traits>& os, deleted_t const&)
{
	return os << "(deleted)";
}

namespace { deleted_t deleted; }

namespace detail {
struct make_deleted
{
  template <class> struct result { typedef field_type type; };

  field_type operator() (field_name const& name) const
  { 
    return field_type ("", ""); 
  }
};
} // namespace detail

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
class attributes_map 
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
   * @param fields fileds definition list.
   * @param parent parent attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  attributes_map (
    std::initializer_list<field_type> fields, 
    proxy_ptr parent = proxy_ptr ())
  : proxy_ (new proxy)
  {
  	proxy_->parent = parent;
  	proxy_->map.insert (fields);
  }
#endif // YAMAIL_CPP >= 11
  
  /// Constructs attributes map from field_list.
  /**
   * @param fields fileds definition list.
   * @param parent parent attributes_map.
   */
  attributes_map (field_list const& fields, 
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

  /// Replaces of adds given field.
  /**
   * @param field the fields to replace or add.
   * @returns reference to this attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  inline attributes_map&
  replace (field_type const& field)
  {
  	proxy_->map[std::get<0> (field)] = std::get<1> (field);
    return *this;
  }

#if YAMAIL_CPP >= 11
  /// Replaces of adds given fields.
  /**
   * @param fields list of fields to replace or add.
   * @returns reference to this attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  attributes_map&
  replace (std::initializer_list<field_type> fields)
  {
  	for (auto const& field: fields)
    {
    	replace (field);
    }

    return *this;
  }
#endif // YAMAIL_CPP >= 11

  /// Replaces of adds given fields.
  /**
   * @param fields list of fields to replace or add.
   * @returns reference to this attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  attributes_map&
  replace (field_list const& fields)
  {
		BOOST_FOREACH (field_type const& field, fields)
    {
    	replace (field);
    }

    return *this;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Erase.

  /// Erase given field from internal map.
  /**
   * @param names field name to be erased.
   * @returns reference to this attributes_map.
   */
  inline attributes_map&
  erase (field_name const& name)
  {
    proxy_->map.erase (name);
    return *this;
  }

#if YAMAIL_CPP >= 11
  /// Removes given fields from internal map.
  /**
   * @param names list of the names.
   * @returns reference to this attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  attributes_map&
  erase (std::initializer_list<field_name> names)
  {
  	for (auto const& name: names)
    	erase (name);

    return *this;
  }
#endif // YAMAIL_CPP >= 11

  /// Removes given fields from internal map.
  /**
   * @param names list of the names.
   * @returns reference to this attributes_map.
   */
  attributes_map&
  erase (name_list const& names)
  {
		BOOST_FOREACH (field_name const& name, names)
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
   * @param to_add fields list to be added into scoped map.
   * @param to_del fields names to be deleted from scoped map.
   * @returns new scoped map instance.
   * @note This method is only defined in C++11 and above compile mode.
   */
  attributes_map
  scoped_change (std::initializer_list<field_type> to_add,
    std::initializer_list<field_name> to_del = 
        std::initializer_list<field_name> ()) const 
  {
  	attributes_map tmp (to_add, proxy_);

  	boost::copy (
  	  to_del | boost::adaptors::transformed (
  	    [] (field_name name) { return field_type (std::move (name), deleted); }
  	  ),
  	  YAMAIL_FQNS_UTILITY::updater (tmp.proxy_->map)
  	);

  	return tmp;
  }
#endif // YAMAIL_CPP >= 11

  /// Creates new scoped attributes_map.
  /** 
   * @param to_add fields list to be added into scoped map.
   * @param to_del fields names to be deleted from scoped map.
   * @returns new scoped map instance.
   */
  attributes_map
  scoped_change (field_list const& to_add, 
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
  boost::optional<field_value const&>
  cascade_find (field_name const& name) const
  {
  	for (proxy_ptr proxy = proxy_; proxy; proxy = proxy->parent)
    {
  	  map_type::const_iterator found = proxy->map.find (name);
  	  if (found != proxy->map.end ()) return found->second;
    }

    return boost::optional<field_value const&> ();
  }

  std::set<field_name> 
  cascade_keys ()
  {
  	// TODO: possible optimization is to strip deleted entries from keys.
  	std::set<field_name> keys;

    for (proxy_ptr p = proxy_; p; p = proxy->parent)
    {
    	boost::copy(map->map_ | boost::adaptors::map_keys,
    	    std::inserter (keys, keys.end ()));
    }

    return keys;
  }

 private:
  friend attributes_map scoped (attributes_map& map);
  friend class delete_from;

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

/// Add or replace field in given map.
/**
 * @param map attributes map.
 * @param field field to add or replace.
 * @returns Reference to map.
 */
inline attributes_map&
operator<< (attributes_map& map, field_type const& field)
{
  return map.replace (field); 
}

// Scoped delete
class delete_from
{
public:
	delete_from (attributes_map& am) : map_ (am.proxy_->map) {}

	delete_from const& operator() (field_name const& name) const
	{
		map_[name] = deleted;
  	return *this;
  }

	delete_from const& operator() (name_list const& names) const
	{
		boost::copy (
  	  names | boost::adaptors::transformed (detail::make_deleted ()),
  	  YAMAIL_FQNS_UTILITY::updater (map_)
  	);

  	return *this;
  }

#if YAMAIL_CPP >= 11
  delete_from const& operator() (std::initializer_list<field_name> names) const
  {
  	boost::copy (
  	  names | boost::adaptors::transformed (
  	    [] (field_name name) { return field_type (std::move (name), deleted); }
  	  ),
  	  YAMAIL_FQNS_UTILITY::updater (map_)
  	);
  	return *this;
  }
#endif

  delete_from const& operator<< (field_name const& name) const
  {
    return (*this) (name);
  }

  delete_from const& operator<< (name_list const& names) const
  {
    return (*this) (names);
  }

private:
  attributes_map::map_type& map_;
};


} // namespace typed
#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_LOG_TYPED_H_

#ifndef _YAMAIL_LOG_TYPED_H_
#define _YAMAIL_LOG_TYPED_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>

#include <yamail/compat/shared_ptr.h>

#include <yamail/utility/list_of.h>
#include <yamail/utility/update_iterator.h>
// #include <yamail/utility/in.h>

#include <boost/type_erasure/builtin.hpp>
#include <boost/type_erasure/operators.hpp>
#include <boost/type_erasure/any_cast.hpp>

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
	arg_process,        ///< Represents name of the service (name of the daemon).
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
		"TIME", "PROCESS", "PID", "PPID", "TID", "PRIORITY"
	};

  
	return attributes[ static_cast<int> (f) ];
}

//_constexpr 
inline char const* 
well_known_attr_for_output (well_known_attr_enum f) _noexcept
{
	static _constexpr char const* attributes[] = {
		"timestamp", "PROCESS", "PID", "PPID", "TID", "PRIORITY"
	};

  
	return attributes[ static_cast<int> (f) ];
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>&
operator<< (std::basic_ostream<CharT, Traits>& os, well_known_attr_enum f)
{
	return os << well_known_attr_for_output (f);
}

template <
    typename CharT
  , typename Traits = std::char_traits<CharT>
  , typename Alloc  = std::allocator<CharT>
>
struct attr
{
	typedef std::basic_ostream<CharT, Traits> ostream_type;
	typedef std::basic_string<CharT, Traits, Alloc> string_type;

  typedef boost::type_erasure::any<
    boost::mpl::vector<
        boost::type_erasure::copy_constructible<>
      , boost::type_erasure::typeid_<>
      , boost::type_erasure::ostreamable<ostream_type>
      , boost::type_erasure::relaxed
    >
  > value;

  typedef boost::variant<well_known_attr_enum, string_type> name;

  typedef std::pair<name const, value> type;
}; 

// TODO: Alloc?
template <typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<< (std::basic_ostream<CharT, Traits>& os, 
    typename attr<CharT,Traits>::type const& ft)
{
	return os << '[' << ft.first << "=>" << ft.second << ']';
}

struct deleted_t {};

template <typename T> 
inline bool is_deleted (T const& val)
{
	return 0 != boost::type_erasure::any_cast<deleted_t const*> (&val);
}

template <typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>&
operator<< (std::basic_ostream<CharT, Traits>& os, deleted_t const&)
{
	return os << "(deleted)";
}

namespace { const deleted_t deleted = deleted_t (); }

namespace detail {

template <typename C, typename Tr, typename A>
struct basic_make_deleted
{
	typedef typename attr<C,Tr,A>::name attr_name;
	typedef typename attr<C,Tr,A>::type attr_type;

  template <class> struct result { typedef attr_type type; };

#if YAMAIL_CPP >= 11
  inline attr_type 
  operator() (attr_name&& name) const
  { 
    return attr_type (std::move (name), deleted); 
  }
#endif

  inline attr_type 
  operator() (attr_name const& name) const
  { 
    return attr_type (name, deleted); 
  }
};
} // namespace detail

#if YAMAIL_CPP >= 11
/// Creates attr from string and value.
/**
 * @param key attr name.
 * @param value attr value.
 * @returns attr definition instance.
 * @note This method is only defined in C++11 and above compile mode.
 */
template <typename C, typename Tr, typename A, typename T>
typename attr<C,Tr,A>::type 
make_attr (std::basic_string<C,Tr,A>&& key, T&& value)
{
	return typename attr<C,Tr,A>::type (std::move (key), std::forward<T> (value));
}

/// Creates attr from string and value.
/**
 * @param key attr name.
 * @param value attr value.
 * @returns attr definition instance.
 * @note This method is only defined in C++11 and above compile mode.
 */
template <typename C, typename Tr, typename A, typename T>
typename attr<C,Tr,A>::type 
make_attr (std::basic_string<C,Tr,A> const& key, T&& value)
{
	return typename attr<C,Tr,A>::type (key, std::forward<T> (value));
}

#else

/// Creates attr from string and value.
/**
 * @param key attr name.
 * @param value attr value.
 * @returns attr definition instance.
 */
template <typename C, typename Tr, typename A, typename T>
typename attr<C,Tr,A>::type 
make_attr (std::basic_string<C,Tr,A> const& key, T const& value)
{
	return typename attr<C,Tr,A>::type (key, value);
}
#endif

/// Creates attr from string and value.
/**
 * @param key attr name.
 * @param value attr value.
 * @returns attr definition instance.
 */
template <typename C, typename T>
typename attr<C>::type 
make_attr (C const *key, T const& value)
{
	return make_attr (std::basic_string<C> (key), value);
}

/// Creates attr from well known key enum ID and value.
/**
 * @param key attr enum ID.
 * @param value attr value.
 * @returns attr definition instance.
 */
template <typename C, typename Tr, typename A, typename T>
typename attr<C,Tr,A>::type 
basic_make_attr (well_known_attr_enum key, T const& value)
{
	return typename attr<C,Tr,A>::type (key, value);
}

/// Creates attr from well known key enum ID and value.
/**
 * @param key attr enum ID.
 * @param value attr value.
 * @returns attr definition instance.
 */
template <typename T>
attr<char>::type 
make_attr (well_known_attr_enum key, T const& value)
{
	return attr<char>::type (key, value);
}

/// Creates attr from well known key enum ID and value.
/**
 * @param key attr enum ID.
 * @param value attr value.
 * @returns attr definition instance.
 */
template <typename T>
attr<wchar_t>::type 
make_wattr (well_known_attr_enum key, T const& value)
{
	return attr<wchar_t>::type (key, value);
}

#if YAMAIL_CPP >= 11
/// Delete helper function.
/**
 * @param key attr name.
 * @returns 'deleter' attr definition instance.
 * @note This method is only defined in C++11 and above compile mode.
 */
template <typename C, typename Tr, typename A>
inline typename attr<C,Tr,A>::type
delete_attr (std::basic_string<C,Tr,A>&& key)
{
	return typename attr<C,Tr,A>::type (std::move (key), deleted);
}
#endif

/// Delete helper function.
/**
 * @param key attr name.
 * @returns 'deleter' attr definition instance.
 * @note This method is only defined in C++11 and above compile mode.
 */
template <typename C, typename Tr, typename A>
inline typename attr<C,Tr,A>::type
delete_attr (std::basic_string<C,Tr,A> const& key)
{
	return typename attr<C,Tr,A>::type (key, deleted);
}

/// Delete helper function.
/**
 * @param key attr name.
 * @returns 'deleter' attr definition instance.
 * @note This method is only defined in C++11 and above compile mode.
 */
template <typename C>
inline typename attr<C>::type
delete_attr (C const* key)
{
	return delete_attr (std::basic_string<C> (key));
}

/// Delete helper function.
/**
 * @param key attr enum ID.
 * @returns 'deleter' attr definition instance.
 */
template <typename C, typename Tr, typename A>
inline typename attr<C,Tr,A>::type 
basic_delete_attr (well_known_attr_enum key)
{
	return typename attr<C,Tr,A>::type (key, deleted);
}

/// Delete helper function.
/**
 * @param key attr enum ID.
 * @returns 'deleter' attr definition instance.
 */
inline attr<char>::type 
delete_attr (well_known_attr_enum key)
{
	return attr<char>::type (key, deleted);
}

/// Delete helper function.
/**
 * @param key attr enum ID.
 * @returns 'deleter' attr definition instance.
 */
inline attr<wchar_t>::type 
delete_wattr (well_known_attr_enum key)
{
	return attr<wchar_t>::type (key, deleted);
}

/// Typed log attributes map.
template <
    typename C
  , typename Tr = std::char_traits<C>
  , typename A = std::allocator<C>
>
class basic_attributes_map;

// forward declaration of friend function. 
// it will be defined after class basic_attributes_map below.
template <typename C, typename Tr, typename A>
basic_attributes_map<C,Tr,A>
scoped (basic_attributes_map<C,Tr,A>& map);

/// Typed log attributes map.
template < typename C , typename Tr, typename A>
class basic_attributes_map 
{
public:
  typedef typename attr<C,Tr,A>::type attr_type;
  typedef typename attr<C,Tr,A>::name attr_name;
  typedef typename attr<C,Tr,A>::value attr_value;

  typedef YAMAIL_FQNS_UTILITY::list_of<attr_type> attr_list;
  typedef YAMAIL_FQNS_UTILITY::list_of<attr_name> name_list;

protected:
  typedef detail::basic_make_deleted<C,Tr,A> make_deleted;

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
   * @param parent parent basic_attributes_map.
   */
  basic_attributes_map (proxy_ptr parent = proxy_ptr ())
  : proxy_ (new proxy)
  {
    proxy_->parent = parent;
  }

#if YAMAIL_CPP >= 11
  /// Constructs attributes map from initiaizer_list.
  /**
   * @param attrs fileds definition list.
   * @param parent parent basic_attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  basic_attributes_map (
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
   * @param parent parent basic_attributes_map.
   */
  basic_attributes_map (attr_list const& attrs, 
      proxy_ptr parent = proxy_ptr ())
  : proxy_ (new proxy)
  {
    proxy_->parent = parent;
#if YAMAIL_CPP < 11
		BOOST_FOREACH (attr_type const& attr, attrs)
#else
		for (attr_type const& attr: attrs)
#endif
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
   * @returns reference to this basic_attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  inline basic_attributes_map&
  replace (attr_type&& attr)
  {
  	proxy_->map[std::move (attr.first)] = std::move (attr.second);
    return *this;
  }
#endif

  /// Replaces of adds given attr.
  /**
   * @param attr the attrs to replace or add.
   * @returns reference to this basic_attributes_map.
   */
  inline basic_attributes_map&
  replace (attr_type const& attr)
  {
  	proxy_->map[attr.first] = attr.second;
    return *this;
  }

#if YAMAIL_CPP >= 11
  /// Replaces of adds given attrs.
  /**
   * @param attrs list of attrs to replace or add.
   * @returns reference to this basic_attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  basic_attributes_map&
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
   * @returns reference to this basic_attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  basic_attributes_map&
  replace (attr_list const& attrs)
  {
#if YAMAIL_CPP < 11
		BOOST_FOREACH (attr_type const& attr, attrs)
#else
		for (attr_name const& attr: attrs)
#endif
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
   * @returns reference to this basic_attributes_map.
   */
  inline basic_attributes_map&
  erase (attr_name const& name)
  {
    proxy_->map.erase (name);
    return *this;
  }

#if YAMAIL_CPP >= 11
  /// Removes given attrs from internal map.
  /**
   * @param names list of the names.
   * @returns reference to this basic_attributes_map.
   * @note This method is only defined in C++11 and above compile mode.
   */
  basic_attributes_map&
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
   * @returns reference to this basic_attributes_map.
   */
  basic_attributes_map&
  erase (name_list const& names)
  {
#if YAMAIL_CPP < 11
		BOOST_FOREACH (attr_name const& name, names)
#else
		for (attr_name const& name: names)
#endif
    {
      erase (name);
    }

    return *this;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Scoped Change.

#if YAMAIL_CPP >= 11
  /// Creates new scoped basic_attributes_map.
  /** 
   * @param to_add attrs list to be added into scoped map.
   * @param to_del attrs names to be deleted from scoped map.
   * @returns new scoped map instance.
   * @note This method is only defined in C++11 and above compile mode.
   */
  basic_attributes_map
  scoped_change (std::initializer_list<attr_type> to_add,
    std::initializer_list<attr_name> to_del = 
        std::initializer_list<attr_name> ()) const 
  {
  	basic_attributes_map tmp (to_add, proxy_);

  	boost::copy (
  	  to_del | boost::adaptors::transformed (
  	    [] (attr_name&& name) { return attr_type (std::move (name), deleted); }
  	  ),
  	  YAMAIL_FQNS_UTILITY::updater (tmp.proxy_->map)
  	);

  	return tmp;
  }
#endif // YAMAIL_CPP >= 11

  /// Creates new scoped basic_attributes_map.
  /** 
   * @param to_add attrs list to be added into scoped map.
   * @param to_del attrs names to be deleted from scoped map.
   * @returns new scoped map instance.
   */
  basic_attributes_map
  scoped_change (attr_list const& to_add, 
      name_list to_del = name_list ()) const
  {
  	basic_attributes_map tmp (to_add, proxy_);

  	boost::copy (
  	  to_del | boost::adaptors::transformed (make_deleted ()),
  	  YAMAIL_FQNS_UTILITY::updater (tmp.proxy_->map)
  	);
  	return tmp;
  }

protected:
  // template <typename C, typename Tr, typename A>
  friend inline boost::optional<attr_value const&>
  cascade_find (basic_attributes_map const& map, attr_name const& name)
  {
    return map.cascade_find (name);
  }

  friend inline std::set<attr_name>
  cascade_keys (basic_attributes_map const& map)
  {
  	return map.cascade_keys ();
  }

  boost::optional<attr_value const&>
  cascade_find (attr_name const& name) const
  {
  	for (proxy_ptr proxy = proxy_; proxy; proxy = proxy->parent)
    {
  	  typename map_type::const_iterator found = proxy->map.find (name);
  	  if (found != proxy->map.end ()) return found->second;
    }

    return boost::optional<attr_value const&> ();
  }

  std::set<attr_name> 
  cascade_keys () const
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
  friend basic_attributes_map scoped<> (basic_attributes_map& map);

  proxy_ptr proxy_;
};

typedef basic_attributes_map<char>    attributes_map;
typedef basic_attributes_map<wchar_t> wattributes_map;

/// Create new scope from given map.
/**
 * @param map attributes map.
 * @returns new scope based on map.
 */
template <typename C, typename Tr, typename A>
inline basic_attributes_map<C,Tr,A>
scoped (basic_attributes_map<C,Tr,A>& map)
{
	return basic_attributes_map<C,Tr,A> (map.proxy_);
}

/// Add or replace attr in given map.
/**
 * @param map attributes map.
 * @param attr attr to add or replace.
 * @returns Reference to map.
 */
template <typename C, typename Tr, typename A>
inline basic_attributes_map<C,Tr,A>&
operator<< (basic_attributes_map<C,Tr,A>& map, 
    typename attr<C,Tr,A>::type const& attr)
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

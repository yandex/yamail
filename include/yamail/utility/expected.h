// http://www.hyc.io/boost/expected-proposal.pdf
#ifndef _YAMAIL_UTILITY_EXPECTED_H_
#define _YAMAIL_UTILITY_EXPECTED_H_
#include <yamail/config.h>
#include <yamail/utility/namespace.h>

# include <yamail/compat/exception.h>
# include <boost/variant.hpp>
# include <type_traits>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace utility {
#else
YAMAIL_FQNS_UTILITY_BEGIN
#endif // GENERATING_DOCUMENTATION

namespace detail {

class holder;

template <typename T>
class wrap
{
public:
  wrap (T const& t) : t (t), rv (false) {}
	wrap (T const& t, bool rv) : t (t), rv (rv) {}

	T&& rget () const { return std::move (const_cast<T&> (t)); }

  template <typename U>
  U cast () const
  {
    return static_cast<U> (t);
  }

  T move () const
  {
    if (rv) return rget ();
    else    return t;
  }

private:
  
	T const& t;
	bool rv;
};

template <typename T>
class expected_wrap : public wrap<T>
{
public:
  expected_wrap (T const& t) : wrap<T> (t, false) {}
  expected_wrap (T& t) : wrap<T> (t, false) {}
  expected_wrap (T&& t) : wrap<T> (t, true) {}
};

template <typename T>
class unexpected_wrap : public wrap<T>
{
public:
  unexpected_wrap (T const& t) : wrap<T> (t, false) {}
  unexpected_wrap (T& t) : wrap<T> (t, false) {}
  unexpected_wrap (T&& t) : wrap<T> (t, true) {}
};

template <typename T> struct is_wrapper 
{ static const bool value = false; };

template <typename T> struct is_wrapper<expected_wrap<T> >
{ static const bool value = true; };

template <typename T> struct is_wrapper<unexpected_wrap<T> >
{ static const bool value = true; };

struct in_place_t {};
struct unexpect_t {};

} // namespace detail

namespace {
_constexpr detail::in_place_t const in_place = detail::in_place_t ();
_constexpr detail::unexpect_t const unexpect = detail::unexpect_t ();
}

template <typename T, typename E = compat::exception_ptr>
class expected;

template <typename E = compat::exception_ptr>
class unexpected;

template <typename T> struct is_expected
{ static const bool value = false; };

template <typename T, typename E> struct is_expected<expected<T,E> >
{ static const bool value = true; };

template <typename T, typename E>
class expected
{
private:
  struct enabler {};

public:
  typedef T value_type;
  typedef E error_type;

  template <typename U>
  struct rebind {
  	typedef expected<U, error_type> type;
  };

  // X.Y.9.1, constructors
  _constexpr expected () _noexcept
    : has_value_ (true), var_ ()
  {}

  expected (expected const& ex)
    : has_value_ (ex.has_value_), var_ (ex.var_)
  {}

#if defined (YAMAIL_USE_RVALUES) && YAMAIL_USE_RVALUES
  expected (expected&& ex) _noexcept
    : has_value_ (ex.has_value_), var_ (std::move (ex.var_))
  {}
#endif

	_constexpr expected (value_type const& t) 
	  : has_value_ (true), var_ (t) {}

#if defined (YAMAIL_USE_RVALUES) && YAMAIL_USE_RVALUES
	_constexpr expected (value_type&& value) 
	  : has_value_ (true), var_ (std::move (value)) {}
#endif

#if YAMAIL_CPP >= 11
	template <typename... Args>
	_constexpr explicit expected (detail::in_place_t tag, Args&&... args)
	  : has_value_ (true)
	  , var_ (value_type (std::forward<Args> (args)...))
	{}

	template <typename... Args>
	_constexpr explicit expected (detail::unexpect_t tag, Args&&... args)
	  : has_value_ (false)
	  , var_ (error_type (std::forward<Args> (args)...))
	{}
#endif

	constexpr expected (detail::unexpected_wrap<error_type> const& wrap)
	  : var_ (wrap.move ())
	{}

  template <typename Err>
	constexpr expected (detail::unexpected_wrap<Err> const& wrap)
	  : var_ (wrap.template cast<error_type> ())
	{}

	constexpr expected (detail::expected_wrap<value_type> const& wrap)
	  : var_ (wrap.move ())
	{}

  template <typename Err>
	constexpr expected (detail::expected_wrap<Err> const& wrap)
	  : var_ (wrap.template cast<value_type> ())
	{}

	// X.Y.9.2, destructor
	~expected () {}

	// X.Y.9.3, assignment

	expected& operator= (expected const& ex)
	{
		has_value_ = ex.has_value_;
		var_ = ex.var_;
    return *this;
  }

#if defined (YAMAIL_USE_RVALUES) && YAMAIL_USE_RVALUES
	expected& operator= (expected&& ex)
	{
		has_value_ = ex.has_value_;
		var_ = std::move (ex.var_);
    return *this;
  }
#endif

#if defined (YAMAIL_USE_RVALUES) && YAMAIL_USE_RVALUES
  template <
      typename U
	  , class = typename std::enable_if< !detail::is_wrapper<U>::value>::type
	>
	expected& operator= (U&& u)
	{
    has_value_ = true;
    var_ = std::forward<U> (u);
    return *this;
  }
#else
  template <typename U>
	expected& operator= (U const& u,
	    typename boost::enable_if_c< 
	        !detail::is_wrapper<U>::value>
	      , enabler
	    >::type = enabler ()
	)
	{
    has_value_ = true;
    var_ = static_cast<value_type> (u);
    return *this;
  }
#endif

  expected& 
  operator= (detail::unexpected_wrap<error_type> const& wrap) _noexcept
  {
  	var_ = wrap.move ();
  	return *this;
  }

  template <typename Err>
  expected& operator= (detail::unexpected_wrap<Err> const& wrap)
  {
  	var_ = wrap.template cast<Err> ();
  	return *this;
  }

  expected& 
  operator= (detail::expected_wrap<value_type> const& wrap) _noexcept
  {
  	var_ = wrap.move ();
  	return *this;
  }

  template <typename Val>
  expected& operator= (detail::expected_wrap<Val> const& wrap)
  {
  	var_ = wrap.template cast<Val> ();
  	return *this;
  }

#if YAMAIL_CPP >= 11
	template <typename... Args>
	void emplace (Args&&... args)
	{
	  has_value_ = true;
	  var_ = value_type (std::forward<Args> (args)...);
	}

  // ???
	template <typename U, typename... Args>
	void emplace (std::initializer_list<U> u, Args&&... args)
	{
	  has_value_ = true;
	  var_ = value_type (std::forward<Args> (args)...);
	}
#endif

	// X.Y.9.4, swap
	void swap(expected& ex) noexcept
	{
		std::swap (has_value_, ex.has_value_);
		std::swap (var_, ex.var_);
  }

  // X.Y.9.5, observers
  _constexpr value_type const* operator-> () const
  {
  	return &boost::get<value_type> (var_);
  }
  
  value_type* operator-> ()
  {
  	return &boost::get<value_type> (var_);
  }

  _constexpr value_type const& operator* () const&
  {
  	return boost::get<value_type> (var_);
  }

  value_type& operator* () &
  {
  	return boost::get<value_type> (var_);
  }

  _constexpr value_type&& operator* () &&
  {
  	return std::move (boost::get<value_type> (var_));
  }

  _constexpr explicit operator bool () const _noexcept
  {
  	return has_value_;
  }

  _constexpr value_type const& value () const& 
  {
  	return boost::get<value_type> (var_);
  }

  value_type& value () & 
  {
  	return boost::get<value_type> (var_);
  }

  _constexpr value_type&& value () && 
  {
  	return boost::get<value_type> (var_);
  }

  _constexpr error_type const& error () const& 
  {
  	return boost::get<error_type> (var_);
  }

  error_type& error () & 
  {
  	return boost::get<error_type> (var_);
  }

  _constexpr error_type&& error () && 
  {
  	return boost::get<error_type> (var_);
  }

  _constexpr unexpected<error_type> 
  get_unexpected () const
  {
  	return unexpected<error_type> (error ());
  }

  template <typename Ex>
  bool has_exception () const
  {
  	// true if and only if !(*this) and the stored exception is a base type of
  	// Ex. TODO XXX
  	assert (0 && "not implemented");
    return false;
  }

#if defined (YAMAIL_USE_RVALUES) && YAMAIL_USE_RVALUES
  template <typename U>
  _constexpr value_type 
  value_or (U&& u) const&
  {
  	if (has_value_) return std::move (boost::get<value_type> (var_));
  	else return static_cast<T> (std::forward<U> (u));
  }

  template <typename U>
  value_type value_or (U&& u) &&
  {
  	if (has_value_) return std::move (boost::get<value_type> (var_));
  	else return static_cast<T> (std::forward<U> (u));
  }
#else
  template <typename U>
  _constexpr value_type 
  value_or (U const& u) const
  {
  	if (has_value_) return boost::get<value_type> (var_);
  	else return static_cast<T> (u);
  }
#endif

  template <typename U,
    class = typename std::enable_if<
                is_expected<U>::value &&
                std::is_same<typename U::error_type, E>::value
            >::type
  >
  expected<U,E> 
  unwrap () const&
  {
  	if (has_value_)
  	  return boost::get<value_type> (var_);
  	else 
  		get_unexpected ();
  }

  template <typename U,
    class = typename std::enable_if<
                ! is_expected<U>::value ||
                ! std::is_same<typename U::error_type, E>::value
            >::type
  >
  expected const&
  unwrap () const&
  {
  	return *this;
  }

  template <typename U,
    class = typename std::enable_if<
                is_expected<U>::value &&
                std::is_same<typename U::error_type, E>::value
            >::type
  >
  expected<U,E> 
  unwrap () &&
  {
  	if (bool (*this))
  	  return std::move (**this);
  	else 
  		get_unexpected ();
  }

  template <typename U,
    class = typename std::enable_if<
                ! is_expected<U>::value ||
                ! std::is_same<typename U::error_type, E>::value
            >::type
  >
  expected
  unwrap () &&
  {
  	return std::move (*this);
  }

#if 0

  expected () : base_t ( value_type () ) {}
  expected (T const& t) : base_t (t) {}

  expected (detail::in_place_t const&, T const& t) : base_t (t) {}
  expected (detail::unexpect_t const&, E const& e) : base_t (e) {}

	template <typename EE>
	expected (detail::unexpected_wrap<EE> const& ee)
	  : base_t (ee.template cast<E> ())
	{}


	template <typename EE>
	expected& operator= (detail::unexpected_wrap<EE> const& ee)
	{
		this->base_t::operator= (ee.template cast<E> ());
		return *this;
  }

	template <typename TT>
	expected (detail::expected_wrap<TT> const& tt)
	  : base_t (tt.template cast<T> ())
	{}

	template <typename TT>
	expected& operator= (detail::expected_wrap<TT> const& tt)
	{
		this->base_t::operator= (tt.template cast<T> ());
		return *this;
  }
#endif

private:
  bool has_value_;
  boost::variant<value_type, error_type> var_;
};


#if defined (YAMAIL_USE_RVALUES) && YAMAIL_USE_RVALUES
template <typename E>
detail::expected_wrap<E>
make_expected (E&& e) 
{ 
	return detail::expected_wrap<E> (std::forward<E> (e));
}

template <typename E>
detail::unexpected_wrap<E>
make_unexpected (E&& e) 
{ 
	return detail::unexpected_wrap<E> (std::forward<E> (e));
}
#endif

#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_UTILITY_END 
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_UTILITY_EXPECTED_H_

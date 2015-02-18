/** @file */
#ifndef _YAMAIL_LOG_COMPAT_H_
#define _YAMAIL_LOG_COMPAT_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>

#include <boost/move/move.hpp>
#include <boost/type_traits.hpp>

#include <boost/mpl/if.hpp>

#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/type_info_wrapper.hpp>
#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/detail/embedded_string_type.hpp>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace log {
#else
  YAMAIL_FQNS_LOG_BEGIN
#endif // GENERATING_DOCUMENTATION

namespace typed {

namespace compat {

namespace mpl = ::boost::mpl;
using namespace ::boost::log;
//! Attribute value manipulator
template< typename RefT >
class add_value_manip
{
public:
  //! Stored reference type
  typedef RefT reference_type;
  //! Attribute value type
  typedef typename boost::remove_cv< 
    typename boost::remove_reference< reference_type >::type 
  >::type value_type;

private:
  typedef typename boost::remove_reference< 
    reference_type 
  >::type& lvalue_reference_type;

  typedef typename mpl::if_<
      boost::is_scalar< value_type >,
      value_type,
      lvalue_reference_type
  >::type stored_type;

  typedef typename mpl::if_<
      boost::is_scalar< value_type >,
      value_type,
      reference_type
  >::type get_value_result_type;

private:
  //! Attribute value
  stored_type m_value;
  //! Attribute name
  attribute_name m_name;

public:
  //! Initializing constructor
  add_value_manip(attribute_name const& name, reference_type value) 
    : m_value(static_cast< lvalue_reference_type >(value)), m_name(name)
  {
  }

  //! Returns attribute name
  attribute_name get_name() const { return m_name; }

  //! Returns attribute value
  get_value_result_type get_value() const 
  { return static_cast< get_value_result_type >(m_value); }
};



template< typename T >
inline add_value_manip< T const& > 
add_value(attribute_name const& name, T const& value)
{
  return add_value_manip< T const& >(name, value);
}

using namespace ::boost::log::attributes;

template< typename T >
class attribute_value_impl :
    public attribute_value::impl
{
public:
    //! Value type
    typedef T value_type;

private:
    //! Attribute value
    const value_type m_value;

public:
    /*!
     * Constructor with initialization of the stored value
     */
    explicit attribute_value_impl(value_type const& v) : m_value(v) {}
    /*!
     * Constructor with initialization of the stored value
     */
    explicit attribute_value_impl(BOOST_RV_REF(value_type) v) : m_value(v) {}

    /*!
     * Attribute value dispatching method.
     *
     * \param dispatcher The dispatcher that receives the stored value
     *
     * \return \c true if the value has been dispatched, \c false otherwise
     */
    virtual bool dispatch(type_dispatcher& dispatcher)
    {
        type_dispatcher::callback< value_type > callback = 
          dispatcher.get_callback< value_type >();
        if (callback)
        {
            callback(m_value);
            return true;
        }
        else
            return false;
    }

    /*!
     * \return The attribute value type
     */
    type_info_wrapper get_type() const 
    { return type_info_wrapper(typeid(value_type)); }

    /*!
     * \return Reference to the contained value.
     */
    value_type const& get() const { return m_value; }
};

/*!
 * The function creates an attribute value from the specified object.
 */
#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

template< typename T >
inline attribute_value make_attribute_value(T&& v)
{
    typedef typename boost::remove_cv< typename boost::remove_reference< T >::type >::type value_type;
    return attribute_value(new attribute_value_impl< value_type >(boost::forward< T >(v)));
}

#else // !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

template< typename T >
inline attribute_value make_attribute_value(T const& v)
{
    typedef typename boost::remove_cv< T >::type value_type;
    return attribute_value(new attribute_value_impl< value_type >(v));
}

template< typename T >
inline attribute_value make_attribute_value(boost::rv< T > const& v)
{
    typedef typename boost::remove_cv< T >::type value_type;
    return attribute_value(new attribute_value_impl< value_type >(v));
}

#endif // !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

/*!
 * The function constructs a \c constant attribute containing the provided value.
 * The function automatically converts C string arguments to \c std::basic_string objects.
 */
template< typename T >
inline constant<
    typename boost::log::aux::make_embedded_string_type<
        typename boost::remove_reference< T >::type
    >::type
> make_constant(BOOST_FWD_REF(T) val)
{
    typedef typename boost::log::aux::make_embedded_string_type<
        typename boost::remove_reference< T >::type
    >::type value_type;
    return constant< value_type >(boost::forward< T >(val));
}

template< typename T >
inline boost::shared_ptr< constant <
    typename boost::log::aux::make_embedded_string_type<
        typename boost::remove_reference< T >::type
    >::type
> > make_shared_constant(BOOST_FWD_REF(T) val)
{
    typedef typename boost::log::aux::make_embedded_string_type<
        typename boost::remove_reference< T >::type
    >::type value_type;
    return boost::make_shared< value_type > (
        constant< value_type >(boost::forward< T >(val)));
}


} // namespace compat
} // namespace typed

#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

namespace boost { namespace BOOST_LOG_NAMESPACE {

using namespace yamail::log::typed::compat;

//! The operator attaches an attribute value to the log record
template< typename CharT, typename RefT >
inline basic_record_ostream< CharT >& 
operator<< (basic_record_ostream< CharT >& strm, 
  add_value_manip< RefT > const& manip)
{
  typedef typename boost::log::aux::make_embedded_string_type< 
    typename add_value_manip< RefT >::value_type 
  >::type value_type;
  attribute_value value(
      new attribute_value_impl< value_type >(manip.get_value()));

  strm.get_record().attribute_values().insert(manip.get_name(), value);
  return strm;
}

} }

#endif // _YAMAIL_LOG_COMPAT_H_


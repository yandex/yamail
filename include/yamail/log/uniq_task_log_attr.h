#ifndef _YAMAIL_LOG_UNIQ_TASK_LOG_ATTR_H_
#define _YAMAIL_LOG_UNIQ_TASK_LOG_ATTR_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>

#include <string>
#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_value.hpp>

#if BOOST_VERSION < 105400
#include <boost/log/attributes/basic_attribute_value.hpp>
#else
#include <boost/log/attributes/attribute_value_impl.hpp>
#endif

YAMAIL_NS_BEGIN
YAMAIL_NS_LOG_BEGIN

#if BOOST_VERSION < 104600
class uniq_task_attr : public boost::log::attribute
{
    typedef boost::log::attributes::basic_attribute_value<std::string> attribute_value_type;
public:
    explicit uniq_task_attr (const std::string& ini = "");

    boost::shared_ptr< boost::log::attribute_value > get_value()
    {
        return boost::shared_ptr< boost::log::attribute_value >(new attribute_value_type(value_));
    }

    const std::string& uniq_id() const
    { return value_; }

private:
    std::string value_;
};
#else
class uniq_task_attr : public boost::log::attribute
{
protected:
  //! Attribute factory implementation
  struct impl : public boost::log::attribute::impl
  {
    impl (std::string& val, const std::string& ini);

    boost::log::attribute_value get_value ()
    {
#if BOOST_VERSION < 105400
      typedef boost::log::attributes::basic_attribute_value<std::string> result_value;
#else
      typedef boost::log::attributes::attribute_value_impl<std::string> result_value;
#endif
      return boost::log::attribute_value(new result_value(value_));
    }

    std::string value_;
  };
public:
  explicit uniq_task_attr (const std::string& ini = "")
  {
    boost::intrusive_ptr< impl > ptr (new impl(value_, ini));
    this->set_impl (ptr);
  }
  std::string uniq_id () const
  { return value_; }

private:
  std::string value_;
};
#endif

YAMAIL_NS_LOG_END
YAMAIL_NS_END

#endif // _YAMAIL_LOG_UNIQ_TASK_LOG_ATTR_H_

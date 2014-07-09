#ifndef _YAMAIL_ERROR_ERROR_H_
#define _YAMAIL_ERROR_ERROR_H_
#include <yamail/config.h>
#include <yamail/namespace.h>
#include <yamail/compat/system.h>


#include <boost/exception_ptr.hpp>
#include <boost/exception/all.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/info_tuple.hpp>
#include <boost/system/system_error.hpp>
#include <boost/tuple/tuple.hpp>

#include <exception>
#include <string>

YAMAIL_FQNS_BEGIN

struct tag_throw_what {};
struct tag_throw_private {};
struct tag_throw_public {};

typedef ::boost::error_info<struct tag_throw_class,std::string> 
    error_class_info;
typedef ::boost::error_info<struct tag_throw_public,std::string> 
    error_public_info;
typedef ::boost::error_info<struct tag_throw_private,std::string> 
    error_private_info;
typedef ::boost::tuple<error_class_info, error_public_info, error_private_info> 
    error_info;

struct tag_throw_error_code {};

typedef ::boost::error_info<struct tag_throw_error_code,int> error_code_info;

// TO THINK: use variant<std|boost::error_code> ?
typedef ::boost::error_info<struct tag_throw_system, 
    YAMAIL_FQNS_COMPAT::error_code> system_error;

// generic error class
class error
  : public ::boost::exception
  , public ::std::exception
{
public:
  inline error (void) 
  { 
  	*this << 
  	  error_info (std::string ("yamail::error"), 
  	              std::string ("error"), std::string ()); 
  }

  explicit inline error (const std::string& cl) 
  {
    *this << error_info (cl, std::string ("error"), std::string ());
  }

  inline error (std::string const& cl, std::string const& pub) 
  {
    *this << error_info (cl, pub, std::string ());
  }

  inline error (std::string const& cl, std::string const& pub, 
                std::string const& prv) 
  {
    *this << error_info (cl, pub, prv);
  }

  virtual inline ~error () throw () {}

  virtual const char* what() const throw();

  std::string error_class () const throw ();
  std::string error_public () const throw ();
  std::string error_private () const throw ();

  virtual std::string public_message () const;
  virtual std::string private_message () const;

  virtual ::boost::exception_ptr exception_ptr() const;

protected:
  std::string message_suffix () const;

  std::string public_message_i () const;
  std::string private_message_i () const;
};

#define YAMAIL_ERROR_DEF(name,base,cl,xpub) public base {                     \
    explicit inline name () : base (cl,xpub) {}                               \
    inline name (std::string const& pub) : base (cl, pub) {}                  \
    inline name (std::string const& pub, std::string const& prv)              \
        : base (cl,pub,prv) {}                                                \
    inline name (std::string const& c, std::string const& pub,                \
                  std::string const& prv) : base (c, pub,prv) {}              \
    virtual inline ~name () throw () {}                                       \
    virtual ::boost::exception_ptr exception_ptr() const                      \
    { return ::boost::copy_exception (*this); }                               \
}

struct deadline_error :
  YAMAIL_ERROR_DEF (deadline_error, error, 
      "yamail::error", "no free workers");

struct std_error :
  YAMAIL_ERROR_DEF (std_error, error, "yamail::error", "std error");

#ifndef BOOST_ERROR_INFO
# define BOOST_ERROR_INFO ::boost::throw_function(BOOST_CURRENT_FUNCTION) <<  \
          ::boost::throw_file(__FILE__) << ::boost::throw_line(__LINE__)      
#endif

YAMAIL_FQNS_END
#endif // _YAMAIL_ERROR_ERROR_H_

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

#if defined(GENERATING_DOCUMENTATION)
namespace yamail {
#else
YAMAIL_FQNS_BEGIN
#endif // GENERATING_DOCUMENTATION

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

////////////////////////////////////////////////////////////////////////////////
/// Generic error class.
/** Error is derived class from boost::exception and std::exception.
 * boost::exception is streaming extended attributes (such as information about
 * functions, files, lines and private messages). 
 * std::exception is used as standard c++ exception interface.
 */
class error
  : public ::boost::exception
  , public ::std::exception
{
public:
  /// Constructs an error instance.
  /**
   * This constructor creates an error instance with class
   * <tt>yamail::error</tt>.
   */
  inline error (void) 
  { 
  	*this << error_info (std::string ("yamail::error"), 
  	              std::string ("error"), std::string ()); 
  }

  /// Constructs an error instance.
  /**
   * This constructor creates an error instance with given class.
   * @param cl The error class.
   */
  explicit inline error (const std::string& cl) 
  {
    *this << error_info (cl, std::string ("error"), std::string ());
  }

  /// Constructs an error instance.
  /**
   * This constructor creates an error instance with given class and
   * \em public message.
   * @param cl The error class.
   * @param pub The public error message.
   */
  inline error (std::string const& cl, std::string const& pub) 
  {
    *this << error_info (cl, pub, std::string ());
  }

  /// Constructs an error instance.
  /**
   * This constructor creates an error instance with given class and
   * \em public and \em private messages.
   * @param cl The error class.
   * @param pub The public error message.
   * @param prv The private error message.
   */
  inline error (std::string const& cl, std::string const& pub, 
                std::string const& prv) 
  {
    *this << error_info (cl, pub, prv);
  }

  /// Destructor.
  /** 
   * Destructs error instance.
   */
  virtual inline ~error () throw () {}

  /// Get string identifying exception.
  /**
   * Returns a null terminated character sequence that may be used to identify
   * the exception.
   *
   * @returns A pointer to a c-string with content related to the exception. 
   *          This is guaranteed to be valid at least until the exception object
   *          from which it is obtained is destroyed or until a non-const member
   *          function of the exception object is called.
   */
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

#if defined(GENERATING_DOCUMENTATION)
}
#else
YAMAIL_FQNS_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_ERROR_ERROR_H_

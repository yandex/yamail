#ifndef _YAMAIL_ERROR_ERROR_H_
#define _YAMAIL_ERROR_ERROR_H_
#include <yamail/config.h>
#include <yamail/namespace.h>
#include <yamail/compat/system.h>
#include <yamail/compat/exception.h>
#include <yamail/compat/move.h>

#include <boost/exception_ptr.hpp>
#include <boost/exception/all.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/info_tuple.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/system/error_code.hpp>

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

#if defined(HAVE_STD_SYSTEM) && HAVE_STD_SYSTEM
typedef ::boost::error_info<struct tag_throw_system, 
    std::error_code
> std_system_error;

std_system_error 
system_error (std::error_code const& ec)
{
  return std_system_error (ec);
}

#if YAMAIL_USE_RVALUES
std_system_error 
system_error (std::error_code&& ec)
{
  return std_system_error (std::move (ec));
}
#endif // YAMAIL_USE_RVALUES

#endif

typedef ::boost::error_info<struct tag_throw_system, 
    boost::system::error_code
> boost_system_error;

boost_system_error 
system_error (boost::system::error_code const& ec)
{
	return boost_system_error (ec);
}

#if YAMAIL_USE_RVALUES
boost_system_error 
system_error (boost::system::error_code&& ec)
{
	return boost_system_error (YAMAIL_FQNS_COMPAT::move (ec));
}
#endif // YAMAIL_USE_RVALUES

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
  virtual const char* what() const throw()
  {
  	try { throw_this (); }
  	catch (boost::exception const& e)
  	{
  		if (std::string const* msg = 
  			  boost::get_error_info <error_public_info> (e))
  			return msg->c_str ();
    }
    catch (...) {}
    return "";
  }

  std::string error_class () const throw ()
  {
  	try { throw_this (); }
  	catch (boost::exception const& e)
  	{
  		try {
  			if (std::string const* msg = 
  				  boost::get_error_info <error_class_info> (e))
  				return *msg;
      }
      catch (...) {}
    }
    catch (...) {}
    return std::string ();
  }

  std::string error_public () const throw ()
  {
  	try { throw_this (); }
  	catch (boost::exception const& e)
  	{
  		try {
  			if (std::string const* msg = 
  				  boost::get_error_info <error_public_info> (e))
  				return *msg;
      }
      catch (...) {}
    }
    catch (...) {}
    return std::string ();
  }

  std::string error_private () const throw ()
  {
  	try { throw_this (); }
  	catch (boost::exception const& e)
  	{
  		try {
  			if (std::string const* msg = 
  				  boost::get_error_info <error_private_info> (e))
  				return *msg;
      }
      catch (...) {}
    }
    catch (...) {}
    return std::string ();
  }

  virtual std::string public_message () const
  {
  	return public_message_i (); //  + message_suffix ();
  }


  virtual std::string private_message () const
  {
  	return private_message_i () + message_suffix ();
  }

  virtual YAMAIL_FQNS_COMPAT::exception_ptr exception_ptr() const
  { 
    return YAMAIL_FQNS_COMPAT::make_exception_ptr(*this);
  }

protected:
  std::string message_suffix () const
  {
  	std::string msg;

  	try { throw_this (); }
  	catch (boost::exception const& e)
  	{
#if defined(HAVE_STD_SYSTEM) && HAVE_STD_SYSTEM
  		// system error
  		if (std::error_code const* se =
  			  boost::get_error_info <std_system_error> (e))
      {
      	msg += ": ";
      		msg += se->message ();
      }
#endif
  		// boost system error
      if (boost::system::error_code const* se =
          boost::get_error_info <boost_system_error> (e))
      {
        msg += ": ";
#if 0 // do not want include asio headers here...
        if (*se == ::boost::asio::error::operation_aborted)
          msg += "Timeout";
        else
#endif
          msg += se->message ();
      }

      char const* const* file = 
          boost::get_error_info < ::boost::throw_file> (e);
      int const* line = boost::get_error_info < ::boost::throw_line> (e);
      // char const* const* func = 
      //    boost::get_error_info < ::boost::throw_function> (e);

      if (file && *file)
      {
      	msg += ", at ";
      	msg += *file;

      	if (line) 
        {
        	msg += '@';
        	msg += ::boost::lexical_cast<std::string> (*line);
        }

        // XXX function names are too long to be returned or maybe not?
      }
    }
    catch (...) {}

    return msg;
  }

  std::string public_message_i () const
  {
  	return error_public ();
  }

  std::string private_message_i () const
  {
  	std::string msg (public_message_i ());
  	std::string prv (error_private ());

  	if (! prv.empty ())
    {
    	msg += ": ";
    	msg += prv;
    }

    return msg;
  }

private:
  void throw_this () const
  {
  	// surprise!
  	throw *this;
  }
};



#define YAMAIL_ERROR_DEF(name,base,cl,xpub) public base {                     \
    explicit inline name () : base (cl,xpub) {}                               \
    inline name (std::string const& pub) : base (cl, pub) {}                  \
    inline name (std::string const& pub, std::string const& prv)              \
        : base (cl,pub,prv) {}                                                \
    inline name (std::string const& c, std::string const& pub,                \
                  std::string const& prv) : base (c, pub,prv) {}              \
    virtual inline ~name () throw () {}                                       \
    virtual YAMAIL_FQNS_COMPAT::exception_ptr exception_ptr() const           \
    { return YAMAIL_FQNS_COMPAT::make_exception_ptr(*this); }                 \
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

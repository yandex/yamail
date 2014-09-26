#ifndef _YAMAIL_UTILITY_COPY_TEST_H_
#define _YAMAIL_UTILITY_COPY_TEST_H_
#include <yamail/config.h>
#include <yamail/utility/namespace.h>

#include <iostream>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace utility {
#else
YAMAIL_FQNS_UTILITY_BEGIN
#endif // GENERATING_DOCUMENTATION

class copy_test
{
  static int id () 
  {
    static int id_;
    return ++id_;
  }

  int id_;

public:
  copy_test () : id_ (id ()) { print ("CTOR_DEFAULT"); }
  copy_test (int i) : id_ (id ()) { print ("CTOR_INT"); }
  template <typename U> copy_test (U) : id_ (id ()) { print ("CTOR_U"); }
  copy_test (copy_test const& x) : id_ (id ()) { print ("CTOR_COPY", x); }

  copy_test& operator= (copy_test const& x)
  {
    print ("ASSIGN_COPY", x);
    return *this;
  }

#if YAMAIL_USE_RVALUES
  copy_test (copy_test&& x) : id_ (id ()) { print ("CTOR_MOVE", x); }
  copy_test& operator= (copy_test&& x)
  {
    print ("ASSIGN_MOVE", x);
    return *this;
  }
#endif

  ~copy_test ()
  {
    print ("DTOR");
  }

  int ping () const
  {
    print ("PING");
    return id_;
  }

private:
  void print_helper (std::string const& s) const
  {
    std::cout << "ID:" << id_ << ": " 
      << (void*) this;
    
    if (! s.empty ()) 
      std::cout << ": " << s;
  }

  void print (std::string const& s) const
  {
    print_helper (s); 
    std::cout << "\n";
  }

  void print (std::string const& s, copy_test const& x) const
  {
    print_helper (s); 
    std::cout << " <=== ";
    x.print_helper ("");
    std::cout << "\n";
  }
};


#if defined(GENERATING_DOCUMENTATION)
}}
#else
YAMAIL_FQNS_UTILITY_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_UTILITY_COPY_TEST_H_

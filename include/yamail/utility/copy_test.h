#ifndef _YAMAIL_UTILITY_COPY_TEST_H_
#define _YAMAIL_UTILITY_COPY_TEST_H_
#include <yamail/config.h>
#include <yamail/utility/namespace.h>

#include <iostream>

YAMAIL_NS_BEGIN
YAMAIL_NS_UTILITY_BEGIN

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
  copy_test (copy_test&& x) : id_ (id ()) { print ("CTOR_MOVE", x); }

  copy_test& operator= (copy_test const& x)
  {
    print ("ASSIGN_COPY", x);
    return *this;
  }

  copy_test& operator= (copy_test&& x)
  {
    print ("ASSIGN_MOVE", x);
    return *this;
  }

  ~copy_test ()
  {
    print ("DTOR");
  }

  int ping () const
  {
    print ("PING");
    return id_;
  }

  int i = 0;

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

YAMAIL_NS_UTILITY_END
YAMAIL_NS_END

#endif // _YAMAIL_UTILITY_COPY_TEST_H_

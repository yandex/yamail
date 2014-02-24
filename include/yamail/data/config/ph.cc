#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/home/phoenix/function/function.hpp>

#include <iostream>

namespace px = boost::phoenix;
using namespace boost::phoenix::arg_names;

struct func 
{
  template <typename A0>
  struct result 
  {
    typedef void type;
  };

/*
  template <typename Arg>
  void operator() (Arg i) const
  { }
*/

  inline void operator() (int i) 
  {
    std::cout << "hello: " << i << "!\n";
  }
};


int main ()
{
  func ff;
  px::function<func> fx (ff);

  int i = 5;
  fx (arg1) (i);


}

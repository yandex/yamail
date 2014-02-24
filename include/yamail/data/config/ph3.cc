#include <boost/phoenix.hpp>

#include <iostream>

#include <yamail/utility/copy_test.h>

namespace px = boost::phoenix;
using namespace boost::phoenix::arg_names;

template <typename A, typename B>
struct TTT
{
  TTT ()
  {
    std::cout << __PRETTY_FUNCTION__ << "\n";
  }
};


struct func 
{
#if 1
  typedef void result_type;
#else
  template <typename Sig> struct result;

  template <typename This, typename C>
  struct result<This (C&, C const&)>
  {
    typedef void type;
  };
#endif

  template <typename A0, typename A1>
  inline void
  operator() (A0&& s, A1&& i) const
  {
    std::cout << __PRETTY_FUNCTION__ << "\n";
    s += i;
    std::cout << "hello: " << i << "!\n";
  }
};


int main ()
{
  px::function<func> fx;

  int i = 5;
  int sum = 0;
  fx (arg1, arg2) (sum, 5);

  std::cout << "sum = " << sum << "\n";

}

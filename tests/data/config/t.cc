#include <string>
#include <iostream>
#include <yamail/utility/in.h>
#include <yamail/utility/copy_test.h>

using namespace y::utility;
using namespace std;

struct A {
  A () {}
//  string s;
};

void foo2 (rw_in<A> a) {}
void foo (rw_in<A> const& a)
{
//  a->i = 66;
}

#include <boost/timer/timer.hpp>
int main ()
{
//  copy_test const a;

  {
    boost::timer::auto_cpu_timer t;
    for (unsigned long i=0; i<1000000000; ++i) foo (A ());
  }

  {
    boost::timer::auto_cpu_timer t;
    for (unsigned long i=0; i<1000000000; ++i) foo2 (A ());
  }
//  cout << "a.i = " << a.i << "\n";
}

#include <iostream>
using namespace std;

#include <yamail/utility/copy_test.h>
#include <yamail/utility/in.h>

struct Cache 
{
  Cache (int) {};
};

using y::utility::copy_test;
using y::utility::rw_in;

void impl (copy_test& c)
{
  cout << __PRETTY_FUNCTION__ << "\n";
}

void parse (rw_in<copy_test> tc)
{
  cout << "&&: " << __PRETTY_FUNCTION__ << "\n";
  impl (tc.get ());
}

int main ()
{
//  copy_test c = { 0 };
  parse ( copy_test (0) );
}

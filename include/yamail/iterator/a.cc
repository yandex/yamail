#include <stdlib.h>
#include <iostream>

struct A 
{
	mutable bool b;
  bool check () const { abort (); return true; }

  bool xx (A const& a)
  {
  	if (a.b && !(a.b = a.check ()))
  		return true;
  	
  	return false;
  }
};

int main ()
{
  A a; a.b = false;
  std::cout << a.xx (a) << "\n";
}

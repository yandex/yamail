struct A { typedef char x; };
struct B { typedef int x; };
struct C { typedef double x; };

#include <iostream>
template <class V = A>
void foo (typename V::x a, V b = V ())
{
  std::cout << __PRETTY_FUNCTION__ << "\n";
  std::cout << a << "\n";
}

int main ()
{
  char aa = 'a'; int bb = 1; double cc = 2.2; 
  foo (cc, C());
}

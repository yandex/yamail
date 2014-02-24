template <class X, class Y, class Z>
struct A 
{
};

struct B : A<int, int, char>
{
};

#include <type_traits>
#include <iostream>

template <typename T> struct is_A : std::false_type {};
template <class X, class Y, class Z> 
struct is_A<A<X,Y,Z> > : std::true_type {};

template <typename T>
typename std::enable_if<! is_A<T>::value>::type
foo (T t)
{
  std::cout << "1: " << __PRETTY_FUNCTION__ << "\n";
}

template <class T>
typename std::enable_if<is_A<T>::value>::type
foo (T t)
{
  std::cout << "2: " << __PRETTY_FUNCTION__ << "\n";
}

int main ()
{
  int i; foo (i);
  A<char,char,char> a;
  B b;
  foo (a);
  foo (b);
}

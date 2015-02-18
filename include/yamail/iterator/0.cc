#include <iostream>
#include <deque>
#include "boundary_iterator.h"

template <int N>
struct counter_n
{
	int n_ = 0;

	void increment () { ++n_; }

	template <typename T> void dereference (T const& t) const {}

	bool is_eof () const { return n_ >= N; }
};

template <
    typename C
  , typename Tr=std::char_traits<C>
  , typename A=std::allocator<C>
>
struct read_until 
{
	using string = std::basic_string<C,Tr,A>;
	string pattern;
  mutable std::deque<typename string::const_iterator> pos;

	read_until (string s) : pattern (std::move (s)) {}
	read_until () {}

	void increment () {
std::cout << "inc\n";
		for (auto& p: pos) ++p;
  }

	void dereference (C c) const
	{

std::cout << "deref: " << c << "\n";
		for (auto i=pos.begin (); i != pos.end (); ++i)
    {
  	  assert (*i != pattern.end ());
  	  if (**i == c)
      {
      	pos.erase (pos.begin (), i);
      	break;
      }
    }

    if (c == pattern[0])
    {
std::cout << "push\n";
      pos.push_back (pattern.begin ());
    }
	}

	bool is_eof () const 
	{
		for (auto const& p: pos) 
			if (p == pattern.end ()) 
      {
		std::cout << "is_eof = true\n";
      	return true;
      }

		std::cout << "is_eof = false\n";
		return false; 
	}
};

template <typename C, typename Tr, typename A>
read_until<C,Tr,A>
make_read_until (std::basic_string<C,Tr,A> p)
{
  return read_until<C,Tr,A> (std::move (p));
}

template <typename C>
read_until<C>
make_read_until (C const* p)
{
  return read_until<C> (p);
}

int main ()
{
	using namespace y::iterator;
	std::string s ("1234567890");

#if 0
	boundary_iterator<std::string::const_iterator, counter_n<2> >
	  b (s.begin ()), e (s.end (), eof);

  for (; b != e; ++b) std::cout << *b << ' ';
  std::cout << '\n';
#endif

  auto b2 = make_boundary_iterator (s.begin (), make_read_until ("567"));
  auto e2 = make_boundary_iterator (s.begin (), eof, make_read_until (""));

  for (; b2 != e2; ++b2) std::cout << *b2 << ' ';
  std::cout << '\n';

}

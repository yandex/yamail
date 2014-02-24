#include <yamail/data/config/detail/include_opener.h>
#include <yamail/data/config/detail/include_cache.h>

#include <iostream>
struct Parser 
{
  template <class AST, class PATH, class ITER, class FS>
  bool operator() (AST&& ast, PATH&& path, ITER&& i1, ITER&& i2, FS&& fs)
  {
    std::cout << "opening: " << path << "\n";
    // std::cout << boost::make_iterator_range (i1, i2) << "\n";;
    return true;
  }
};

using namespace y::data::config::detail;

#include <boost/timer/timer.hpp>

int main ()
{
  Parser p;
  boost_fs_handler<> fs;
  include_cache<Parser> io (p, fs);

  boost::timer::auto_cpu_timer t;
  for (unsigned int i=0; i<100000; ++i)
    io ("include_opener.cc");


}

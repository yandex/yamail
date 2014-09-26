#include <iostream>
#include <yamail/utility/fast_function.h>
#include <yamail/utility/benchmark.h>


int main ()
{
  int x{0};
  for(int xx = 0; xx < 5; ++xx)
  {
  	{
      start_benchmark();
      for(int i = 0; i < 1000000000; ++i)
      {
        std::function<int(int)> t2 = [&x](int i){ return i + x; };
        std::function<void(int)> t1 = [&x, &t2](int i){ x = t2(i); };
        for(int i = 0; i < 1000000000; ++i) t1(i);
      }
      std::cout << "[std::func]" << end_benchmark() << std::endl;
    }

  	{
      start_benchmark();
      for(int i = 0; i < 1000000000; ++i)
      {
        y::utility::fast_function<int(int)> t2 = [&x](int i){ return i + x; };
        y::utility::fast_function<void(int)> t1 = [&x, &t2](int i){ x = t2(i); };
        for(int i = 0; i < 1000000000; ++i) t1(i);
      }
      std::cout << "[y::utility::fast_func]" << end_benchmark() << std::endl;
    }
  }
}

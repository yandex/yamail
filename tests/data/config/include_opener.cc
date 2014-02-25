#include <yamail/data/config/detail/include_opener.h>
#include <yamail/data/config/detail/include_cache.h>

#include <system_error>

#include <iostream>
struct Saver 
{
  template <class AST, class PATH, class ITER, class FS>
  bool operator() (AST&& /*ast*/, PATH&& /*path*/, 
      ITER&& first, ITER&& last, 
      FS&& /*fs*/)
  {
    // std::cout << "opening: " << path << "\n";
    // std::cout << boost::make_iterator_range (first, last) << "\n";;
    content.assign (first, last);
    return true;
  }

  std::string content;
};

// #include <boost/timer/timer.hpp>
#include <gtest/gtest.h>

#if !defined(INPUT_FILE_NAME)
# define INPUT_FILE_NAME "/data/config/include_opener.cc"
#endif

#define XSTR(s) STR(s)
#define STR(s) #s

TEST(IncludeOpenerTest, HandlesOpenSingleFile) {
  using namespace y::data::config::detail;
  Saver sv;
  boost_fs_handler<> fs;
  include_opener<Saver> io (sv, fs);

  // boost::timer::auto_cpu_timer t;
  // for (unsigned int i=0; i<100000; ++i)
  include_opener<Saver>::ast_ptr aptr;
  // std::cout << "CONTENT EMPTY () = " << sv.content << "\n";
  ASSERT_NO_THROW (aptr = io (TESTS_DIR INPUT_FILE_NAME));
  EXPECT_FALSE (sv.content.empty ());
}

TEST(IncludeCacheTest, HandlesOpenSingleFile) {
  using namespace y::data::config::detail;
  Saver sv;
  boost_fs_handler<> fs;
  include_cache<Saver> ic (sv, fs);

  // boost::timer::auto_cpu_timer t;
  // for (unsigned int i=0; i<100000; ++i)
  include_cache<Saver>::ast_ptr aptr;
  // std::cout << "CONTENT EMPTY () = " << sv.content << "\n";
  ASSERT_NO_THROW (aptr = ic (TESTS_DIR INPUT_FILE_NAME));
  EXPECT_FALSE (sv.content.empty ());
}

TEST(IncludeTest, ComparesOpenerAndCached) {
  using namespace y::data::config::detail;
  Saver sv1, sv2;
  boost_fs_handler<> fs;
  include_opener<Saver> io (sv1, fs);
  include_cache<Saver> ic (sv2, fs);

  // boost::timer::auto_cpu_timer t;
  // for (unsigned int i=0; i<100000; ++i)
  include_cache<Saver>::ast_ptr aptr;
  // std::cout << "CONTENT EMPTY () = " << sv.content << "\n";
  ASSERT_NO_THROW (aptr = io (TESTS_DIR INPUT_FILE_NAME));
  ASSERT_NO_THROW (aptr = ic (TESTS_DIR INPUT_FILE_NAME));
  EXPECT_FALSE (sv1.content.empty ());
  EXPECT_EQ (sv1.content, sv2.content);
}

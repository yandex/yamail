/*
 * streambuf-test.cc
 *
 *  Test suite for the basic_streambuf class functionality
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <yamail/data/zerocopy/streambuf.h>

namespace {
using namespace testing;
using namespace yamail::data::zerocopy;

struct StreambufTest : public Test {
    /**
     * Stream buffer wrapper to access protected members for diagnostic
     */
    struct Streambuf : public basic_streambuf<> {
        typedef basic_streambuf<> Base;
        Streambuf() {}
#define __UNPROTECT_CONST_MEMBER0( type, name) type name() const { return Base::name(); }
        __UNPROTECT_CONST_MEMBER0( char_type *, gptr);
        __UNPROTECT_CONST_MEMBER0( char_type *, egptr);
        __UNPROTECT_CONST_MEMBER0( char_type *, eback);
        __UNPROTECT_CONST_MEMBER0( char_type *, epptr);
        __UNPROTECT_CONST_MEMBER0( char_type *, pbase);
        __UNPROTECT_CONST_MEMBER0( char_type *, pptr);
    };
};


TEST_F(StreambufTest, constructor) {
    Streambuf s;
}

TEST_F(StreambufTest, constructor_setsEbackGptrEgptr_areEqual) {
    Streambuf s;
    ASSERT_EQ(s.eback(), s.gptr());
    ASSERT_EQ(s.gptr(), s.egptr());
    ASSERT_EQ(s.eback(), s.egptr());
}

TEST_F(StreambufTest, constructor_setsPutSize_intoFragmentSize) {
    Streambuf s;
    ASSERT_EQ( std::size_t(std::distance(s.pbase(), s.epptr())), Streambuf::default_min_fragmentation);
}

TEST_F(StreambufTest, begin_isEqualTo_gptr) {
    Streambuf s;
    ASSERT_EQ(&(*(s.begin())), s.gptr());
}

TEST_F(StreambufTest, end_isEqualTo_pptr) {
    Streambuf s;
    ASSERT_EQ(&(*(s.begin())), s.pptr());
}

}


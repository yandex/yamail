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
    static const std::size_t fragmentSize;
    /**
     * Stream buffer wrapper to access protected members for diagnostic
     */
    struct Streambuf : public basic_streambuf<> {
        typedef basic_streambuf<> Base;
        Streambuf() : Base(16, 32, 1, fragmentSize, 1024) {}
#define __UNPROTECT_CONST_MEMBER0( type, name) type name() const { return Base::name(); }
#define __UNPROTECT_MEMBER1( type, name, type0 ) type name( type0 arg0 ) { return Base::name(arg0); }
        __UNPROTECT_CONST_MEMBER0( char_type *, gptr);
        __UNPROTECT_CONST_MEMBER0( char_type *, egptr);
        __UNPROTECT_CONST_MEMBER0( char_type *, eback);
        __UNPROTECT_CONST_MEMBER0( char_type *, epptr);
        __UNPROTECT_CONST_MEMBER0( char_type *, pbase);
        __UNPROTECT_CONST_MEMBER0( char_type *, pptr);
        __UNPROTECT_CONST_MEMBER0( const fragment_list &, fragments);
        __UNPROTECT_MEMBER1( void, reserve, std::size_t );
    };

    std::string string(const boost::asio::const_buffer& b) const {
        return std::string(boost::asio::buffer_cast<const char*>(b),
                    boost::asio::buffer_size(b));
    }
};

const std::size_t StreambufTest::fragmentSize = 16;


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
    ASSERT_EQ( std::size_t(std::distance(s.pbase(), s.epptr())), fragmentSize);
}

TEST_F(StreambufTest, begin_isEqualTo_gptr) {
    Streambuf s;
    ASSERT_EQ(&(*(s.begin())), s.gptr());
}

TEST_F(StreambufTest, end_isEqualTo_pptr) {
    Streambuf s;
    ASSERT_EQ(&(*(s.begin())), s.pptr());
}

TEST_F(StreambufTest, reserve_withSizeMoreThanFragment_addAnotherFragments) {
    Streambuf s;
    s.reserve(fragmentSize + 1);
    ASSERT_EQ(s.fragments().size(), 2);
}

TEST_F(StreambufTest, reserve_withSizeLessOrEqualToFragment_addNoFragment) {
    Streambuf s;
    s.reserve(fragmentSize);
    ASSERT_EQ(s.fragments().size(), 1);
}

TEST_F(StreambufTest, reserve_withSizeMoreThanMaxSize_throwsException) {
    Streambuf s;
    ASSERT_THROW(s.reserve(2048), std::length_error);
}

TEST_F(StreambufTest, iostreamInterface_withStringOut_inputSameString) {
    Streambuf s;
    std::iostream io(&s);
    io << "0123456789ABCDEF0123456789ABCDEF!";
    std::string str;
    io >> str;
    ASSERT_EQ(str, "0123456789ABCDEF0123456789ABCDEF!");
}

TEST_F(StreambufTest, ostreamInterface_withString_asioBuffersReturnsStringFragments) {
    Streambuf s;
    std::ostream io(&s);
    io << "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF!";
    Streambuf::const_buffers_type buffers(s.data());
    ASSERT_EQ(string(buffers[0]), "0123456789ABCDEF");
    ASSERT_EQ(string(buffers[1]), "0123456789ABCDEF0123456789ABCDEF");
    ASSERT_EQ(string(buffers[2]), "!");
}

TEST_F(StreambufTest, xsputn_withBufferMoreThanReserved_reserveFragments) {
    Streambuf s;
    ASSERT_TRUE(false);
}

TEST_F(StreambufTest, xsputn_withBufferMoreThanFragment_promteToNextFragments) {
    Streambuf s;
    ASSERT_TRUE(false);
}

TEST_F(StreambufTest, xsputn_withNonEmptyBuffer_commitsAllData) {
    Streambuf s;
    ASSERT_TRUE(false);
}

}


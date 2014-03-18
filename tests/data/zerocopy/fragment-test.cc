/*
 * fragment-test.cc
 *
 *  Test suite for fragment.h classes
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <yamail/data/zerocopy/fragment.h>
#include "allocator_mock.h"

namespace {
using namespace testing;
using namespace yamail_100000::data::zerocopy::detail;

struct FragmentTest : public Test {
    FragmentTest() : allocator(allocatorMock), buffSize(buffSizeDefault) {
    }
    typedef AllocatorMock<basic_fragment::byte_t> AllocatorMockInst;
    typedef AllocatorMockWrapper<AllocatorMockInst> Allocator;
    AllocatorMockInst allocatorMock;
    Allocator allocator;
    enum {buffSizeDefault = 64};
    basic_fragment::size_type buffSize;
    basic_fragment::byte_t buff[buffSizeDefault];
};

TEST_F(FragmentTest, basicFragment_defaultCtor_setsBeginIntoZero) {
    basic_fragment fragment;
    EXPECT_EQ(fragment.begin(), basic_fragment::iterator(0));
}

TEST_F(FragmentTest, basicFragment_defaultCtor_setsEndIntoZero) {
    basic_fragment fragment;
    EXPECT_EQ(fragment.end(), basic_fragment::iterator(0));
}

TEST_F(FragmentTest, basicFragment_defaultCtor_setsSizeIntoZero) {
    basic_fragment fragment;
    EXPECT_EQ(fragment.size(), basic_fragment::size_type(0));
}

TEST_F(FragmentTest, basicFragment_ctorWithArguments_setsBeginIntoBufferStart) {
    basic_fragment fragment(buff, buffSize);
    EXPECT_EQ(fragment.begin(), buff);
}

TEST_F(FragmentTest, basicFragment_ctorWithArguments_setsBeginIntoBufferStartPlusSize) {
    basic_fragment fragment(buff, buffSize);
    EXPECT_EQ(fragment.end(), buff + buffSize);
}

TEST_F(FragmentTest, basicFragment_ctorWithArguments_setsSizeIntoBufferSize) {
    basic_fragment fragment(buff, buffSize);
    EXPECT_EQ(fragment.size(), buffSize);
}

TEST_F(FragmentTest, basicFragment_containsWithBegin_returnsTrue) {
    basic_fragment fragment(buff, buffSize);
    EXPECT_TRUE(fragment.contains(fragment.begin()));
}

TEST_F(FragmentTest, basicFragment_containsWithinBeginEnd_returnsTrue) {
    basic_fragment fragment(buff, buffSize);
    EXPECT_TRUE(fragment.contains(fragment.begin()+ buffSize/2));
}

TEST_F(FragmentTest, basicFragment_containsWithEnd_returnsTrue) {
    basic_fragment fragment(buff, buffSize);
    EXPECT_TRUE(fragment.contains(fragment.end()));
}

TEST_F(FragmentTest, basicFragment_containsWithAfterEnd_returnsFalse) {
    basic_fragment fragment(buff, buffSize);
    EXPECT_FALSE(fragment.contains(fragment.end()+1));
}

TEST_F(FragmentTest, basicFragment_containsWithBeforeBegin_returnsFalse) {
    basic_fragment fragment(buff, buffSize);
    EXPECT_FALSE(fragment.contains(fragment.begin()-1));
}

TEST_F(FragmentTest, basicRaiiFragment_ctorAllocate_dtorDeallocate) {
    InSequence s;
    EXPECT_CALL(allocatorMock, allocate(buffSize)).WillOnce(Return(buff));
    EXPECT_CALL(allocatorMock, deallocate(buff,buffSize)).WillOnce(Return());
    basic_raii_fragment<Allocator> fragment(buffSize, allocator);
}

TEST_F(FragmentTest, basicRaiiFragment_begin_returnsAllocatedBufferStart) {
    EXPECT_CALL(allocatorMock, allocate(_)).WillOnce(Return(buff));
    EXPECT_CALL(allocatorMock, deallocate(_,_)).WillOnce(Return());
    basic_raii_fragment<Allocator> fragment(buffSize, allocator);
    EXPECT_EQ(fragment.begin(), buff);
}

TEST_F(FragmentTest, basicRaiiFragment_end_returnsAfterAllocatedBuffer) {
    EXPECT_CALL(allocatorMock, allocate(_)).WillOnce(Return(buff));
    EXPECT_CALL(allocatorMock, deallocate(_,_)).WillOnce(Return());
    basic_raii_fragment<Allocator> fragment(buffSize, allocator);
    EXPECT_EQ(fragment.end(), buff + buffSize);
}

TEST_F(FragmentTest, basicRaiiFragment_size_returnsAllocatedBufferSize) {
    EXPECT_CALL(allocatorMock, allocate(_)).WillOnce(Return(buff));
    EXPECT_CALL(allocatorMock, deallocate(_,_)).WillOnce(Return());
    basic_raii_fragment<Allocator> fragment(buffSize, allocator);
    EXPECT_EQ(fragment.size(), buffSize);
}

}

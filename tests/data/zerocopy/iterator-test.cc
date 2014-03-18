/*
 * iterator-test.cc
 *
 * Test suite for the iterator.h class
 */


#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <yamail/data/zerocopy/iterator.h>
#include "allocator_mock.h"
#include <vector>

namespace {
using namespace testing;
using namespace yamail_100000;
using namespace yamail_100000::data::zerocopy;

struct IteratorTest : public Test {
    typedef detail::fragment Fragment;
    typedef detail::basic_raii_fragment<std::allocator<Fragment::byte_t> > FragmentInstance;
    typedef compat::shared_ptr<Fragment> FragmentPtr;
    typedef std::vector<FragmentPtr> FragmentList;
    typedef iterator<detail::fragment::byte_t, Fragment, FragmentList> Iterator;

    FragmentList fragments;

    enum {fragmentSize = 16};

    FragmentPtr createFragment() const {
        return FragmentPtr(new FragmentInstance(fragmentSize));
    }

    IteratorTest() {
        for(int i=0; i!=3; ++i ) {
            fragments.push_back(createFragment());
        }
    }

    Fragment::iterator begin( std::size_t fragmentNo = 0 ) {
        return fragments[fragmentNo]->begin();
    }

    Fragment::iterator end( std::size_t fragmentNo = 0 ) {
        return fragments[fragmentNo]->end();
    }

};

TEST_F(IteratorTest, constructor_withFirstFragmentBeginPosition_setsPositionToFirstFragmentBegin) {
    Iterator i(fragments, begin());
    EXPECT_EQ(begin(), &(*i));
}

//TEST_F(IteratorTest, constructor_withLastFragmentEndPosition_setsPositionToLastFragmentEnd) {
//    Iterator i(fragments, end(2), true);
//    EXPECT_EQ(end(2), &(*i));
//}

TEST_F(IteratorTest, increment_withinFragment_setsPositionToIncrementedFragmentPosition) {
    Fragment::iterator fragmentIter(begin());
    Iterator i(fragments, fragmentIter);
    EXPECT_EQ(++fragmentIter, &(*++i));
}

TEST_F(IteratorTest, increment_onFragmentBoundary_setsPositionToNextFragmentBegin) {
    Iterator i(fragments, end(0) - 1, true);
    EXPECT_EQ(begin(1), &(*++i));
}

TEST_F(IteratorTest, decrement_withinFragment_setsCurrentPositionRight) {
    Fragment::iterator fragmentIter(begin()+1);
    Iterator i(fragments, fragmentIter);
    EXPECT_EQ(--fragmentIter, &(*--i));
}

TEST_F(IteratorTest, decrement_onFragmentBoundary_setsPositionToPreviousFragmentLast) {
    Iterator i(fragments, begin(1), true);
    EXPECT_EQ(end(0) - 1, &(*--i));
}

TEST_F(IteratorTest, advance_withinFragmentSize_setsPositionToAdvancedFragmentPosition) {
    Fragment::iterator fragmentIter(begin());
    Iterator i(fragments, fragmentIter);
    const Iterator::difference_type adv = fragmentSize/2;
    EXPECT_EQ(fragmentIter+adv, &(*(i+adv)));
}

TEST_F(IteratorTest, advance_moreThanFragmentSize_setsPositionToNextFragment) {
    const Iterator::difference_type adv = fragmentSize+2;
    Iterator i(fragments, begin());
    EXPECT_EQ(begin(1)+2, &(*(i+adv)));
}

TEST_F(IteratorTest, advance_negativeWithinFragmentSize_setsPositionToAdvancedFragmentPosition) {
    Fragment::iterator fragmentIter(end() - 1);
    Iterator i(fragments, fragmentIter);
    const Iterator::difference_type adv = -fragmentSize/2;
    EXPECT_EQ(fragmentIter+adv, &(*(i+adv)));
}

TEST_F(IteratorTest, advance_negativeMoreThanFragmentSize_setsPositionToPreviousFragment) {
    const Iterator::difference_type adv = -(fragmentSize+2);
    Iterator i(fragments, begin(1) + 2, true);
    EXPECT_EQ(begin(0), &(*(i+adv)));
}

TEST_F(IteratorTest, equal_withSameIterators_returnsTrue) {
    EXPECT_TRUE( Iterator(fragments, begin(1), true) == Iterator(fragments, begin(1), true) );
}

TEST_F(IteratorTest, equal_withDifferentIterators_returnsFalse) {
    EXPECT_FALSE( Iterator(fragments, begin(0), true) == Iterator(fragments, begin(1), true) );
}

TEST_F(IteratorTest, distance_withSameIterators_returnsZero) {
    Iterator first(fragments, begin(1), true);
    Iterator last(fragments, begin(1), true);
    EXPECT_EQ( std::distance(first, last), 0 );
}

TEST_F(IteratorTest, distance_withIteratorsDinstance_returnsDistance) {
    Iterator first(fragments, begin(0), true);
    Iterator last(fragments, begin(1), true);
    EXPECT_EQ( std::distance(first, last), fragmentSize );
}

} // namespace


/*
 * iterator-test.cc
 *
 * Test suite for the iterator.h class
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <yamail/data/zerocopy/iterator.h>
#include <list>

namespace {
using namespace testing;
using namespace yamail;
using namespace yamail::data::zerocopy;

struct IteratorTest : public Test {
    typedef detail::fragment Fragment;
    typedef detail::basic_raii_fragment<std::allocator<Fragment::byte_t> > FragmentInstance;
    typedef compat::shared_ptr<Fragment> FragmentPtr;
    typedef std::list<FragmentPtr> FragmentList;
    typedef iterator<Fragment::byte_t, Fragment, FragmentList> Iterator;

    FragmentList fragments;

    enum {fragmentSize = 16};

    FragmentPtr createFragment() const {
        return FragmentPtr(new FragmentInstance(fragmentSize));
    }

    void appendFragment() {
        fragments.push_back(createFragment());
    }

    IteratorTest() {
        for(int i=0; i!=2; ++i ) {
            appendFragment();
        }
    }

    FragmentList::iterator at( std::size_t fragmentNo ) {
        FragmentList::iterator i(fragments.begin());
        while(fragmentNo--) {
            ++i;
        }
        return i;
    }

    Fragment::iterator begin( std::size_t fragmentNo = 0 ) {
        return (*at(fragmentNo))->begin();
    }

    Fragment::iterator end( std::size_t fragmentNo = 0 ) {
        return (*at(fragmentNo))->end();
    }

};

TEST_F(IteratorTest, constructor_withFirstFragmentBeginPosition_setsPositionToFirstFragmentBegin) {
    Iterator i(fragments, begin(), fragments.begin());
    EXPECT_EQ(begin(), &(*i));
}

TEST_F(IteratorTest, constructor_withEmptyFragment_constructsIterator) {
    FragmentList f;
    EXPECT_NO_THROW(Iterator(f, 0));
}

TEST_F(IteratorTest, constructor_withLastFragmentEndPosition_setsPositionToLastFragmentEnd) {
    Iterator i(fragments, end(1));
    EXPECT_EQ(end(1), &(*i));
}

TEST_F(IteratorTest, increment_withinFragment_setsPositionToIncrementedFragmentPosition) {
    Fragment::iterator fragmentIter(begin());
    Iterator i(fragments, fragmentIter);
    EXPECT_EQ(++fragmentIter, &(*++i));
}

TEST_F(IteratorTest, increment_onFragmentBoundary_setsPositionToNextFragmentBegin) {
    Iterator i(fragments, end(0) - 1);
    EXPECT_EQ(begin(1), &(*++i));
}

TEST_F(IteratorTest, decrement_withinFragment_setsPositionToDecrementedOneOfFragment) {
    Fragment::iterator fragmentIter(begin()+1);
    Iterator i(fragments, fragmentIter);
    EXPECT_EQ(--fragmentIter, &(*--i));
}

TEST_F(IteratorTest, decrement_withEndOfFragment_setsPositionToLastOfFragment) {
    Fragment::iterator fragmentIter(end());
    Iterator i(fragments, fragmentIter);
    EXPECT_EQ(--fragmentIter, &(*--i));
}
TEST_F(IteratorTest, decrement_onFragmentBoundary_setsPositionToPreviousFragmentLast) {
    Iterator i(fragments, begin(1));
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

TEST_F(IteratorTest, advance_moreThanAllFragmentSize_setsPositionToLastFragmentEnd) {
    const Iterator::difference_type adv = fragmentSize*2+2;
    Iterator i(fragments, begin());
    EXPECT_EQ(end(1), &(*(i+adv)));
}

TEST_F(IteratorTest, advance_negativeWithinFragmentSize_setsPositionToAdvancedFragmentPosition) {
    Fragment::iterator fragmentIter(end() - 1);
    Iterator i(fragments, fragmentIter);
    const Iterator::difference_type adv = -fragmentSize/2;
    EXPECT_EQ(fragmentIter+adv, &(*(i+adv)));
}

TEST_F(IteratorTest, advance_negativeOneFromFragmentEnd_setsPositionFragmentLast) {
    Fragment::iterator fragmentIter(end());
    Iterator i(fragments, fragmentIter);
    EXPECT_EQ(fragmentIter-1, &(*(i-1)));
}

TEST_F(IteratorTest, advance_negativeMoreThanFragmentSize_setsPositionToPreviousFragment) {
    const Iterator::difference_type adv = -(fragmentSize+2);
    Iterator i(fragments, begin(1) + 2);
    EXPECT_EQ(begin(0), &(*(i+adv)));
}

TEST_F(IteratorTest, equal_withSameIterators_returnsTrue) {
    EXPECT_TRUE( Iterator(fragments, begin(1)) == Iterator(fragments, begin(1)) );
}

TEST_F(IteratorTest, equal_withDifferentIterators_returnsFalse) {
    EXPECT_FALSE( Iterator(fragments, begin(0)) == Iterator(fragments, begin(1)) );
}

TEST_F(IteratorTest, distance_withSameIterators_returnsZero) {
    Iterator first(fragments, begin(1));
    Iterator last(fragments, begin(1));
    EXPECT_EQ( std::distance(first, last), 0 );
}

TEST_F(IteratorTest, distance_withIteratorsDinstance_returnsDistance) {
    Iterator first(fragments, begin(0));
    Iterator last(fragments, begin(1));
    EXPECT_EQ( std::distance(first, last), fragmentSize );
}

TEST_F(IteratorTest, dereference_withLastFragmentEndPositionAfterAppendAnotherFragment_returnsFirstValueOfTheFragment) {
    Iterator i(fragments, end(1));
    appendFragment();
    EXPECT_EQ(begin(2), &(*i));
}

TEST_F(IteratorTest, equal_withSameIteratorsAfterAppendAnotherFragment_returnsTrue) {
    Iterator i1(fragments, end(1));
    appendFragment();
    Iterator i2(fragments, begin(2));
    EXPECT_TRUE(i1 == i2);
}

} // namespace


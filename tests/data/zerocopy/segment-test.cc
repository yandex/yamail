/*
 * segment-test.cc
 *
 * Test suite for segment.h classes
 */


#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <yamail/data/zerocopy/segment.h>
#include "allocator_mock.h"

namespace {
using namespace testing;
using namespace yamail::data::zerocopy;

struct SegmentTest : public Test {
    typedef segment Segment;
    typedef Segment::fragment_type Fragment;
    typedef detail::basic_raii_fragment<std::allocator<Fragment::byte_t> > FragmentInstance;
    typedef Segment::fragment_ptr FragmentPtr;
    typedef std::list<FragmentPtr> FragmentList;

    FragmentList fragments;

    enum {fragmentSize = 16};

    FragmentPtr createFragment() const {
        return FragmentPtr(new FragmentInstance(fragmentSize));
    }

    void appendFragment() {
        fragments.push_back(createFragment());
    }

    SegmentTest() {
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

    bool equal(const Segment & s1, const Segment & s2) const {
        if(s1.size() != s2.size()) {
            return false;
        }
        return equal(s1.begin(), s1.end(), s2.begin());
    }

    template <typename In1, typename In2>
    bool equal(In1 first1, In1 last1, In2 first2) const {
        while( first1!=last1 ) {
            if(&(*first1++) != &(*first2++)) {
                return false;
            }
        }
        return true;
    }
};

TEST_F(SegmentTest, constructor_default_setEmptySegment) {
    Segment s;
    ASSERT_TRUE( s.begin() == s.end() );
}

TEST_F(SegmentTest, empty_withDefaultCtedObject_returnsTrue) {
    Segment s;
    ASSERT_TRUE( s.empty() );
}

TEST_F(SegmentTest, constructor_withRange_setBeginIteratorAtFirstSegmentBegin) {
    Segment s(at(0), at(2), begin(0), end(1));
    ASSERT_EQ( &(*(s.begin())), begin(0) );
}

TEST_F(SegmentTest, constructor_withRange_setEndIteratorAtLastSegmentEnd) {
    Segment s(at(0), at(2), begin(0), end(1));
    ASSERT_EQ( &(*(s.end())), end(1) );
}

TEST_F(SegmentTest, constructor_withContainer_setBeginIteratorAtFirstSegmentBegin) {
    Segment s(fragments, begin(0), end(1));
    ASSERT_EQ( &(*(s.begin())), begin(0) );
}

TEST_F(SegmentTest, constructor_withContainer_setEndIteratorAtLastSegmentEnd) {
    Segment s(fragments, begin(0), end(1));
    ASSERT_EQ( &(*(s.end())), end(1) );
}

TEST_F(SegmentTest, size_withFullFragments_returnSummaryFragmentsSize) {
    Segment s(fragments, begin(0), end(1));
    ASSERT_EQ( s.size(), std::size_t(fragmentSize * 2) );
}

TEST_F(SegmentTest, size_withPartialFragments_returnSummaryFragmentsPartsSize) {
    Segment s(fragments, begin(0)+1, end(1)-1);
    ASSERT_EQ( s.size(), std::size_t(fragmentSize * 2 - 2) );
}

TEST_F(SegmentTest, appendOnNonEmptyObject_withSegmentWithFullFragment_mergeElements) {
    Segment s1(fragments.begin(), ++(fragments.begin()), begin(0), end(0));
    s1.append(Segment(++(fragments.begin()), fragments.end(), begin(1), end(1)));
    Segment s2(fragments, begin(0), end(1));
    ASSERT_EQ( s1.size(), s2.size() );
    ASSERT_TRUE( equal(s1, s2) );
}

TEST_F(SegmentTest, appendOnNonEmptyObject_withSegmentWithPartialFragment_mergeElements) {
    Segment s1(fragments.begin(), ++(fragments.begin()), begin(0), end(0) - 1);
    s1.append(Segment(++(fragments.begin()), fragments.end(), begin(1) + 1, end(1)));
    ASSERT_EQ( s1.size(), std::size_t(fragmentSize - 1) * 2 );
    ASSERT_TRUE( equal(s1.cbegin(), s1.cbegin() + fragmentSize - 1, begin(0)) );
    ASSERT_TRUE( equal(s1.cbegin() + fragmentSize - 1, s1.cend(), begin(1) + 1) );
}

TEST_F(SegmentTest, appendOnNonEmptyObject_withEmptySegment_addsNoElements) {
    Segment s(fragments, begin(0), end(1));
    const std::size_t size = s.size();
    s.append(Segment());
    ASSERT_EQ( s.size(), size );
}

TEST_F(SegmentTest, appendOnEmptyObject_withNonEmptySegment_addsElements) {
    Segment s1;
    Segment s2(fragments, begin(0), end(1));
    s1.append(s2);
    ASSERT_TRUE( equal(s1, s2) );
}

}

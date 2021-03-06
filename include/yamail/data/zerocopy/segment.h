#ifndef _YAMAIL_DATA_ZEROCOPY_SEGMENT_H_
#define _YAMAIL_DATA_ZEROCOPY_SEGMENT_H_

#include <yamail/config.h>
#include <yamail/data/zerocopy/namespace.h>

#include <yamail/data/zerocopy/fragment.h>
#include <yamail/data/zerocopy/iterator.h>

#include <yamail/compat/shared_ptr.h>

#include <memory>
#include <vector>
#include <cstring>

YAMAIL_FQNS_DATA_ZC_BEGIN

template< typename Alloc = std::allocator<void> >
class basic_segment {
public:
    typedef detail::basic_fragment fragment_type;
    typedef compat::shared_ptr<fragment_type> fragment_ptr;
    typedef typename Alloc::template rebind<fragment_ptr>::other allocator_type;
    typedef typename fragment_type::iterator fragment_iterator;
    typedef fragment_type::size_type size_type;
    typedef fragment_type::byte_t byte_t;

    typedef std::vector<fragment_ptr, allocator_type> fragment_list;
    typedef zerocopy::iterator<byte_t const, fragment_type, fragment_list>
            const_iterator;
    typedef zerocopy::iterator<byte_t, fragment_type, fragment_list> iterator;

    basic_segment (allocator_type const& alloc = allocator_type ())
    : fragment_list_(alloc), head_(0), tail_(0) {
    }

    template <typename InputIterator>
    basic_segment ( InputIterator first_frag, InputIterator last_frag,
            fragment_iterator head, fragment_iterator tail,
            allocator_type const& alloc = allocator_type () )
    : fragment_list_(first_frag, last_frag, alloc), head_(head), tail_(tail) {
    }

    template <typename S>
    basic_segment ( S const& seq,
            fragment_iterator head, fragment_iterator tail,
            allocator_type const& alloc = allocator_type () )
    : fragment_list_(seq.begin (), seq.end (), alloc), head_(head), tail_(tail) {
        assert( ! (!seq.empty() && ( !(*(seq.begin()))->contains(head) ||
                !(*--(seq.end()))->contains(tail)) ));
    }

private:
    template <typename T>
    T begin_iter() const {
        return T(fragment_list_, head(), fragment_list_.begin());
    }

    template <typename T>
    T end_iter() const {
        if(empty()) {
            return begin_iter<T>();
        }
        return T(fragment_list_, tail(), --(fragment_list_.end()));
    }
public:

    const_iterator begin () const {
        return begin_iter<const_iterator>();
    }
    const_iterator end () const {
        return end_iter<const_iterator>();
    }
    const_iterator cbegin () const {
        return begin();
    }
    const_iterator cend () const {
        return end();
    }

    iterator begin () {
        return begin_iter<iterator>();
    }
    iterator end () {
        return end_iter<iterator>();
    }

    fragment_ptr wrap_fragment(fragment_iterator data, std::size_t size, fragment_ptr raii) const {
        return fragment_ptr(
                new detail::raii_wrapper_fragment<fragment_ptr>(data, size, raii));
    }

    basic_segment& append(const basic_segment& x) {
        if (x.empty()) {
            return *this;
        }

        if (empty()) {
            return *this = x;
        }

        if (tail() != fragment_list_.back()->end()) {
            fragment_list_.back() = wrap_fragment(
                    fragment_list_.back()->begin(),
                    std::size_t(tail() - fragment_list_.back()->begin()),
                    fragment_list_.back() );
        }

        if (x.head() != x.fragment_list_.front()->begin()) {
            fragment_list_.push_back(wrap_fragment(
                    x.head(),
                    std::size_t(x.fragment_list_.front()->end() - x.head()),
                    x.fragment_list_.front() ));
        } else {
            fragment_list_.push_back(x.fragment_list_.front());
        }

        fragment_list_.insert(fragment_list_.end(),
                ++(x.fragment_list_.begin()), x.fragment_list_.end());

        tail_ = x.tail();
        return *this;
    }

    fragment_iterator head () const {
        return head_;
    }

    fragment_iterator tail () const {
        return tail_;
    }

    size_type size () const {
        return size_type(std::distance(begin(), end()));
    }

    bool empty() const {
        return head() == tail();
    }

    void swap (basic_segment& x) {
        using std::swap;
        swap (fragment_list_, x.fragment_list_);
        swap (head_, x.head_);
        swap (tail_, x.tail_);
    }

private:
    fragment_list fragment_list_;

    fragment_iterator head_;
    fragment_iterator tail_;
};

template <typename A>
inline void swap (basic_segment<A>& x, basic_segment<A>& y) {
    x.swap (y);
}

typedef basic_segment<> segment;

YAMAIL_FQNS_DATA_ZC_END
#endif // _YAMAIL_DATA_ZEROCOPY_SEGMENT_H_

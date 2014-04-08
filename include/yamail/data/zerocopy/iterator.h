#ifndef _YAMAIL_DATA_ZEROCOPY_ITERATOR_H_
#define _YAMAIL_DATA_ZEROCOPY_ITERATOR_H_

#include <yamail/config.h>
#include <yamail/data/zerocopy/namespace.h>

#include <yamail/data/zerocopy/fragment.h>
#include <yamail/compat/shared_ptr.h>

#include <boost/iterator/iterator_facade.hpp>

#include <cassert>
#include <list>
#include <algorithm>
#include <yamail/compat/bind.h>

YAMAIL_FQNS_DATA_ZC_BEGIN

template<typename Value,
    typename Fragment = detail::fragment,
    typename SegmentSeq = std::list<compat::shared_ptr<Fragment> > >
class iterator : public boost::iterator_facade <
                            iterator<Value, Fragment, SegmentSeq>,
                            Value,
                            boost::random_access_traversal_tag
                            > {
    typedef boost::iterator_facade <
                            iterator<Value, Fragment, SegmentSeq>,
                            Value,
                            boost::random_access_traversal_tag
                            > iterator_facade_;

protected:
    typedef SegmentSeq fragment_list;
    //                             of list     of shared_ptr
    typedef typename fragment_list::value_type::element_type fragment_type;

    typedef compat::shared_ptr<fragment_type> fragment_ptr;
    typedef typename fragment_list::const_iterator fragment_iterator;

    typedef typename fragment_type::iterator skip_iterator;

public:
    typedef typename iterator_facade_::difference_type difference_type;

    iterator () : fragment_list_(0), after_last_frag_ (false) {}

    iterator (fragment_list const& fragments, skip_iterator const& pos,
            fragment_iterator fragment )
    : fragment_list_(&fragments),
      fragment_(fragment),
      pos_(pos),
      after_last_frag_(frag_list().empty() || pos_at_end()) {
    }

    iterator (fragment_list const& fragments, skip_iterator const& pos)
    : fragment_list_(&fragments),
      fragment_(std::find_if(fragments.begin(), fragments.end(),
              compat::bind(&fragment_type::contains, _1, pos))),
      pos_(pos),
      after_last_frag_(frag_list().empty() || pos_at_end()) {
        assert(!(fragment_ == frag_list ().end() && !frag_list().empty()));
    }

protected:
    fragment_list const& frag_list () const {
        assert (fragment_list_ != 0);
        return *fragment_list_;
    }

private:

    skip_iterator fine_pos() const {
        return after_last_frag_ && !is_last_fragment() ? next_pos() : pos_;
    }

    bool pos_at_end() const {
        return pos_ == (*fragment_)->end();
    }

    friend class boost::iterator_core_access;

    bool equal_pos(iterator const& other) const {
        if (after_last_frag_ ^ other.after_last_frag_) {
            return fine_pos() == other.fine_pos();
        }
        return pos_ == other.pos_;
    }

    bool equal (iterator const& other) const {
        return equal_pos(other) && fragment_list_ == other.fragment_list_;
    }

    void increment () {
        assert (fragment_ != frag_list ().end ());
        assert (pos_ != (*fragment_)->end () || !is_last_fragment ());

        if (after_last_frag_) {
            assert (!is_last_fragment());
            ++fragment_;
            pos_ = (*fragment_)->begin ();
            after_last_frag_ = false;
        }
        ++pos_;
        if (pos_at_end()) {
            if (is_last_fragment ()) {
                after_last_frag_ = true;
            } else {
                ++fragment_;
                pos_ = (*fragment_)->begin ();
            }
        }
    }

    typename iterator_facade_::reference dereference () const {
        return *fine_pos();
    }

    void decrement () {
        if (after_last_frag_) {
            after_last_frag_ = false;
        } else if (pos_ == (*fragment_)->begin ()) {
            assert (fragment_ != frag_list ().begin ());
            --fragment_;
            pos_ = (*fragment_)->end ();
        }
        --pos_;
    }

    void add_position (difference_type n) {
        assert (n >= 0);

        while (n > ((*fragment_)->end () - pos_)) {
            n -= ((*fragment_)->end () - pos_);
            if (is_last_fragment ()) {
                pos_ = (*fragment_)->end ();
                n = 0;
            } else {
                ++fragment_;
                assert (fragment_ != frag_list ().end ());
                pos_ = (*fragment_)->begin ();
            }
        }

        assert (!(n!=0 && pos_ == (*fragment_)->end () && is_last_fragment ()));
        pos_ += n;
    }

    void substract_position (difference_type n) {
        assert (n <= 0);

        while (n < ((*fragment_)->begin () - pos_)) {
            assert (fragment_ != frag_list ().begin ());
            n -= ((*fragment_)->begin () - pos_);
            --fragment_;
            pos_ = (*fragment_)->end ();
            --pos_;
            ++n;
        }

        pos_ += n;
    }

    void advance (difference_type n) {
        if (n > 0) {
            add_position(n);
        } else {
            substract_position(n);
        }
        after_last_frag_ = pos_at_end();
    }

    difference_type distance_to (iterator const& other) const {
        assert (&frag_list () == &other.frag_list ());

        difference_type result = 0;

        if (fragment_ == other.fragment_) {
            return other.pos_ - pos_;
        }

        if (!is_last_fragment ()) {
            result = (*fragment_)->end () - pos_;
            fragment_iterator f = fragment_;
            for (++f; f != other.fragment_ && f != frag_list().end()/*??*/; ++f) {
                result += (*f)->size();
            }
            if (f == other.fragment_) {
                result += (other.pos_ - (*f)->begin ());
                return result;
            }
        }

        assert (fragment_ != frag_list ().begin ());

        result = pos_ - (*fragment_)->begin ();
        fragment_iterator f = fragment_;
        for (--f; f != other.fragment_; --f) {
            result += (*f)->size();
        }

        assert (f == other.fragment_);

        result += ((*f)->end () - other.pos_);

        return -result;
    }

    skip_iterator next_pos() const {
        assert (!is_last_fragment());
        fragment_iterator i = fragment_;
        ++i;
        return (*i)->begin ();
    }

    bool is_last_fragment () const {
        return &(*fragment_) == &(frag_list ().back ());
    }

    template <typename> friend class basic_segment;
    template <typename, typename, typename, typename>
    friend class basic_streambuf;

    fragment_list const* fragment_list_;

    fragment_iterator fragment_;
    skip_iterator pos_;
    bool after_last_frag_;
};

YAMAIL_FQNS_DATA_ZC_END
#endif // _YAMAIL_DATA_ZEROCOPY_ITERATOR_H_

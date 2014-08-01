#ifndef _YAMAIL_UTILITY_LIST_OF_H_
#define _YAMAIL_UTILITY_LIST_OF_H_
#include <yamail/config.h>
#include <yamail/utility/namespace.h>

#include <yamail/utility/storage.h>

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_convertible.hpp>

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace utility {
#else
YAMAIL_FQNS_UTILITY_BEGIN
#endif // GENERATING_DOCUMENTATION

template <typename T>
struct list_of
{
	struct list 
	{
		T       t;
		list*   next;

		list () : t () {}

		template <typename A0> 
		list (A0 const& a0) 
		: t (a0), next (0) {}

		template <typename A0, typename A1> 
		list (A0 const& a0, A1 const& a1) 
		: t (a0, a1), next (0) {}

		template <typename A0, typename A1, typename A2> 
		list (A0 const& a0, A1 const& a1, A2 const& a2) 
		: t (a0, a1, a2), next (0) {}

	}*                list_;

  template <typename U>
	class iter
	  : public boost::iterator_adaptor<
	        iter<U>
	      , list*
	      , U&
	      , boost::forward_traversal_tag
	    >
	{
	private:
	  struct enabler {};

	public:
		iter (list* l = 0) 
		  : iter::iterator_adaptor_ (l) {}

		template <typename OtherValue>
		iter (
  		  iter<OtherValue> const& other
  		, typename boost::enable_if<
  		      boost::is_convertible<OtherValue*, U*>
  		    , enabler
  		  >::type = enabler ()
  	) 
  	  : iter::iterator_adaptor_ (other.base ()) {}

  private:
    friend class boost::iterator_core_access;

    void increment () { this->base_reference () = this->base ()->next; }

    typename iter::iterator_adaptor_::reference dereference () const
    { return this->base ()->t; }
  };

  typedef       T       value_type;
  typedef       T&      reference;
  typedef const T&      const_reference;
  typedef std::size_t   size_type;
  typedef iter<T      > iterator;
  typedef iter<T const> const_iterator;

  size_type size_;

  /// Number of elements.
  /**
   * @returns number of elements in list.
   */
  _constexpr size_type size () const _noexcept { return size_; }

  /// First element.
  /**
   * @returns first element.
   */
  iterator begin () _noexcept
  {
  	return iterator (list_);
  }

  /// First element.
  /**
   * @returns first element.
   */
  _constexpr const const_iterator begin () const _noexcept
  {
  	return const_iterator (list_);
  }

  /// One past the last element.
  iterator end () _noexcept
  {
  	return iterator ();
  }

  /// One past the last element.
  _constexpr const_iterator end () const _noexcept
  {
  	return const_iterator ();
  }

	_constexpr list_of () : list_ (0), size_ (0) {}

  template <typename A0>
	list_of (A0 const& a0, storage<list> l = storage<list> ())
	: list_ (new (&l.data) list (a0))
	, size_ (0)
	{
		l.created = true;
  }

  template <typename A0, typename A1>
	list_of (A0 const& a0, A1 const& a1, storage<list> l = storage<list> ())
	: list_ (new (&l.data) list (a0, a1))
	, size_ (0)
	{
		l.created = true;
  }

  template <typename A0, typename A1, typename A2>
	list_of (A0 const& a0, A1 const& a1, A2 const& a2,
	    storage<list> l = storage<list> ())
	: list_ (new (&l.data) list (a0, a1, a2))
	, size_ (0)
	{
		l.created = true;
  }

  template <typename A0>
  list_of& operator() (A0 const& a0, storage<list> l = storage<list> ())
	{
		list* tmp = new (&l.data) list (a0);
		l.created = true;

    tmp->next = list_;
    list_ = tmp;

    ++size_;
    return *this;
  }

  template <typename A0, typename A1>
  list_of& operator() (A0 const& a0, A1 const& a1,
	    storage<list> l = storage<list> ())
	{
		list* tmp = new (&l.data) list (a0, a1);
		l.created = true;

    tmp->next = list_;
    list_ = tmp;

    ++size_;
    return *this;
  }

  template <typename A0, typename A1, typename A2>
  list_of& operator() (A0 const& a0, A1 const& a1, A2 const& a2,
	    storage<list> l = storage<list> ())
	{
		list* tmp = new (&l.data) list (a0, a1, a2);
		l.created = true;

    tmp->next = list_;
    list_ = tmp;

    ++size_;
    return *this;
  }

};


#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::utility
#else
YAMAIL_FQNS_UTILITY_END
#endif // GENERATING_DOCUMENTATION


#endif // _YAMAIL_UTILITY_LIST_OF_H_

#ifndef _YAMAIL_DATA_MSGPACK_IO_BUFFER_H_
#define _YAMAIL_DATA_MSGPACK_IO_BUFFER_H_

#include <yamail/config.h>
#include <yamail/data/msgpack/namespace.h>

#include <yamail/compat/shared_ptr.h>

#include <boost/mpl/if.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/utility/enable_if.hpp>

YAMAIL_FQNS_DATA_MSGPACK_BEGIN

namespace detail 
{

template <typename T>
class buffer 
{
  friend inline std::size_t buffer_size (buffer const& buf)
  {
  	return buf.size_;
  }

	template <typename PtrT>
	friend inline PtrT buffer_cast (buffer const& b)
	{
		return static_cast<PtrT> (buf_.data_);
  }

  friend inline buffer& operator+= (buffer& buf, std::size_t sz)
  {
  	assert (offset <= buffer_size (buf));
  	buf.data_ += offset;
  	buf.size_ -= offset;
  	return buf;
  }

  typedef T value_type;
  typedef value_type* pointer;

  pointer     data_;
  std::size_t size_;

  typedef typename boost::mpl::if_<
      boost::is_const<value_type>
    , void const *
    , void       *
  >::type void_pointer;

public:
  buffer () : data_ (0), size_ (0) {}

  buffer (void_pointer data, std::size_t const& size)
    : data_ (static_cast<pointer> (data))
    , size_ (size)
  {
  }

  template <typename OtherT>
  buffer (
      buffer<OtherT> other
    , typename boost::enable_if<
          boost::is_convertible<OtherT, pointer>
        , enabler
      >::type = enabler ()
  )
    : data_ (other.data_)
    , size_ (other.size_)
  {
  }

  template <typename OtherT>
  typename boost::enable_if<
      boost::is_convertible<OtherT, pointer>
    , buffer&
  >::type
  operator= (buffer<OtherT>& other)
  {
  	data_ = other.data_;
  	size_ = other.size_;
  	return *this;
  }
}

} // namespace detail

typedef detail::buffer<char const>   const_buffer;
typedef detail::buffer<char      > mutable_buffer;

using detail::buffer_cast;
using detail::buffer_size;
using detail::operator+=;

template <typename Buffer, typename Seq = std::deque<Buffer> >
class buffer_sequence: public Seq
{
	struct enable {};

	template <typename, typename> friend class buffer_sequence;

public:
  inline buffer_sequence () {}

  template <typename T>
  inline buffer_sequence (T* buffer_owner) : buffer_owner_ (buffer_owner) {}

  template <typename T>
  inline buffer_sequence (
    YAMAIL_FQNS_COMPAT::::shared_ptr<T> const& buffer_owner)
    : buffer_owner_ (buffer_owner)
  {
  }

  template <typename OtherBuffer, typename OtherSeq>
  buffer_sequence (buffer_sequence<OtherBuffer, OtherSeq> const& other
    , typename boost::enable_if<
          boost::is_convertible<OtherBuffer, Buffer>
        , enabler
      >::type = enabler ()
  )
    : Seq (other.begin (), other.end ())
    , buffer_owner_ (other.buffer_owner_)
  {
  }

  template <typename OtherBuffer, typename OtherSeq>
  typename boost::enable_if<
      boost::is_convertible<OtherBuffer, Buffer>
    , buffer_sequence&
  >::type operator= (buffer_sequence<OtherBuffer, OtherSeq> const& other)
  {
  	if (this != reinterpret_cast<buffer_sequence const*> (&other))
    {
    	buffer_owner_.reset ();

    	this->assign (other.begin (), other.end ());
    	buffer_owner_ = other.buffer_owner_;
    }

    return *this;
  }

  const YAMAIL_FQNS_COMPAT::::shared_ptr<void>& buffer_owner () const
  {
  	return buffer_owner_; 
  }

  template <typename O>
  void set_owner (O o)
  {
  	buffer_owner_.reset (o);
  }

  template <typename P>
  void set_owner (YAMAIL_FQNS_COMPAT::::shared_ptr<P> p)
  {
  	buffer_owner_ = p;
  }

private:
  boost:shared_ptr<void> buffer_owner_;
};

typedef buffer_sequence<const_buffer>   const_buffer_sequence;
typedef buffer_sequence<mutable_buffer> mutable_buffer_sequence;

template <typename Buffer>
std::size_t 
buffers_size (buffer_sequence<Buffer> const& buffers)
{
	std::size_t size = 0;

	typedef typename buffer_sequence<Buffer>::const_iterator iterator;
	for (iterator i = buffers.begin (); i != buffers.end (); ++i)
		size += buffer_size (*i);

	return size;
}



YAMAIL_FQNS_DATA_MSGPACK_END
#endif // _YAMAIL_DATA_MSGPACK_IO_BUFFER_H_

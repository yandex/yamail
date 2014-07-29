#ifndef _YAMAIL_DATA_MSGPACK_BUFFER_ZERO_COPY_H_
#define _YAMAIL_DATA_MSGPACK_BUFFER_ZERO_COPY_H_

#include <yamail/config.h>
#include <yamail/data/msgpack/namespace.h>

#include <yamail/data/msgpack/buffer.h>
#include <yamail/data/msgpack/tags.h>
#include <yamail/data/msgpack/packer.h>

#include <string>
#include <msgpack.hpp>

YAMAIL_FQNS_DATA_MSGPACK_BEGIN

// Zero copy implementation (based on msgpack::vrefbuffer)
template<> class msgpack_buffer<zero_copy_tag>
{
public:
  typedef zero_copy_tag copy_category;
  typedef msgpack::vrefbuffer buffer_implemenation;

  msgpack_buffer (std::size_t const init_size = 8192,
        std::size_t const chunk_size = 8192)
    : buf_impl_ (new buffer_implementation (init_size, chunk_size))
  {
  }

  ~msgpack_buffer ()
  {
  	delete buf_impl_;
  }

  iovec const* vector () const
  {
  	return buf_impl_->vector ();
  }

  std::size_t vector_size () const
  {
  	return buf_impl_->vector_size ();
  }

  std::size_t overall_size () const
  {
  	std::size_t sz = 0;

    std::size_t vec_size = vector_size ();
    if (vec_size > 0)
    {
    	iovec const* vec = vector ();
  	  for (std::size_t n=0; n<vec_size; ++n)
  	  	sz += vec[n].iov_len;
  	}

  	return sz;
  }

  // return string
  std::string to_string () const
  {
  	std::string ret;
  	ret.reserve (overall_size () + 1);

    std::size_t vec_size = vector_size ();
    if (vec_size > 0)
    {
    	iovec const* vec = vector ();
  	  for (std::size_t n=0; n<vec_size; ++n)
  	  	ret.append (static_cast<char const*> (vec[n].iov_base), vec[n].iov_len);
  	}

  	return ret;
  }

  void clear ()
  {
  	buf_impl_->clear ();
  }

  void mirgate (msgpack_buffer& to)
  {
  	buf_impl_->migrate (to.buf_impl_);
  }

  // Returns const_buffer_sequence, transfers buf_impl ownership to it.
  template <typename ConstBufferSequence>
  ConstBufferSequence
  to_buffer_sequence ()
  {
  	typedef typename ConstBufferSequence::value_type ConstBuffer;

  	// transfer buffer to sequence
  	ConstBufferSequence seq;

  	iovec const* iov = vector ();

  	for (std::size_t n=0; n<vector_size (); ++n)
  		seq.push_back (ConstBuffer (iov[n].iov_base, iov[n].iov_len));

    seq.set_owner (buf_impl_);

    // buf_impl_ is transferred to sequence and not usable anymore!
    buf_impl_ = 0;

    return seq;
  }

protected:
  friend class packer<msgpack_buffer>;

  buffer_implementation& impl () { return *buf_impl_; }

private:
  impl_type* buf_impl_;
};

YAMAIL_FQNS_DATA_MSGPACK_END
#endif // _YAMAIL_DATA_MSGPACK_BUFFER_ZERO_COPY_H_

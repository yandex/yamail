#ifndef _YAMAIL_DATA_MSGPACK_BUFFER_COPY_H_
#define _YAMAIL_DATA_MSGPACK_BUFFER_COPY_H_

#include <yamail/config.h>
#include <yamail/data/msgpack/namespace.h>

#include <yamail/data/msgpack/buffer.h>
#include <yamail/data/msgpack/tags.h>
#include <yamail/data/msgpack/packer.h>

#include <string>
#include <msgpack.hpp>

YAMAIL_FQNS_DATA_MSGPACK_BEGIN

// Zero copy implementation (based on msgpack::sbuffer)
template<> class msgpack_buffer<copy_tag>
{
public:
  typedef copy_tag copy_category;
  typedef msgpack::sbuffer buffer_implemenation;

  msgpack_buffer (std::size_t const init_size = 8192)
    : buf_impl_ (new buffer_implementation (init_size))
  {
  }

  ~msgpack_buffer ()
  {
  	delete buf_impl_;
  }

  iovec const* vector () const
  {
  	iov_.iov_base = const_cast<char *> (buf_impl_->data ());
  	iov_.iov_len  = buf_impl_->size ();
  	return &iov_;
  }

  _constexpr std::size_t vector_size () const
  {
  	return 1;
  }

  std::size_t overall_size () const
  {
  	return buf_impl_->size ();
  }

  std::string to_string () const
  {
  	return std::string (buf_impl_->data (), buf_impl_->size ());
  }

  void clear ()
  {
  	buf_impl_->clear ();
  }

  void mirgate (msgpack_buffer& to)
  {
  	to.buf_impl_->write (buf_impl_->data (), buf_impl_->size ());
  	clear ();
  }

  // Returns const_buffer_sequence, transfers buf_impl ownership to it.
  template <typename ConstBufferSequence>
  ConstBufferSequence
  to_buffer_sequence ()
  {
  	typedef typename ConstBufferSequence::value_type ConstBuffer;
  	ConstBufferSequence seq;

  	seq.push_back (ConstBuffer (buf_impl_->data (), buf_impl_->size ()));

    seq.set_owner (buf_impl_);

    // buf_impl_ is transferred to sequence and not usable anymore!
    buf_impl_ = 0;

    return seq;
  }

protected:
  friend class packer<msgpack_buffer>;

  buffer_implementation& impl () { return *buf_impl_; }

private:
  mutable iovec iov_;
  impl_type* buf_impl_;
};

YAMAIL_FQNS_DATA_MSGPACK_END
#endif // _YAMAIL_DATA_MSGPACK_BUFFER_COPY_H_

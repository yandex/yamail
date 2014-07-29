#ifndef _YAMAIL_DATA_MSGPACK_UNPACKER_H_
#define _YAMAIL_DATA_MSGPACK_UNPACKER_H_

#include <yamail/config.h>
#include <yamail/data/msgpack/namespace.h>

#include <cstring> // std::memcpy
#include <algorithm> // std::min

#include <msgpack.hpp>

YAMAIL_FQNS_DATA_MSGPACK_BEGIN

template <typename ConstBufferSequence>
class unpacker: public ::msgpack::unpacker 
{
	typedef ConstBufferSequence const_buffer_sequence;
	typedef msgpack::unpacker base_type;

	const_buffer_sequence buffers_;
	typename const_buffer_sequence::const_iterator iter_;
	bool inner_loop_;

public:
#if YAMAIL_CPP >= 11
  unpacker (const_buffer_sequence&& buffers)
    : buffers_ (std::move (buffers))
    , iter_ (buffers_.begin ())
    , inner_loop_ (false)
  {
  	reserve_buffer ();
  }
#endif

  unpacker (const_buffer_sequence const& buffers)
    : buffers_ (buffers)
    , iter_ (buffers_.begin ())
    , inner_loop_ (false)
  {
  	reserve_buffer ();
  }
  
  template <typename T>
  void unpack (T& t)
  {
  	while (inner_loop_ || ! buffers_.empty ())
    {
    	if (! inner_loop_)
      {
      	std::size_t to_copy = std::min (buffer_size (buffers_.front ()),
      	  base_type::buffer_capacity ());

      	std::memcpy (base_type::buffer (),
      	    buffer_cast<void const*> (buffers_.front ()), to_copy);

      	if (to_copy == buffer_size (buffers_.front ()))
      		buffers_.pop_front ();
      	else
      		buffers_.front () += to_copy;

      	base_type::buffer_consumed (to_copy);
      	inner_loop_ = true;
      }

      msgpack::unpacked result;
      if (this->next (&result))
      {
      	msgpack::object obj = result.get ();
      	obj.convert (&t);
      	result.zone ().reset ();

      	return;
      }

      inner_loop_ = false;
    }

    // no data for deserialization.
    throw std::runtime_error ("msgpack unpack error");
    
  }

protected:
  void reserve_buffer ()
  {
  	base_type::reserve_buffer (
  	  std::min (buffers_size (buffers_), std::size_t (32*1024)));
  }
};

YAMAIL_FQNS_DATA_MSGPACK_END
#endif // _YAMAIL_DATA_MSGPACK_UNPACKER_H_

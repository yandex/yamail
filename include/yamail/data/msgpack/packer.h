#ifndef _YAMAIL_DATA_MSGPACK_PACKER_H_
#define _YAMAIL_DATA_MSGPACK_PACKER_H_

#include <yamail/config.h>
#include <yamail/data/msgpack/namespace.h>

#include <msgpack.hpp>

YAMAIL_FQNS_DATA_MSGPACK_BEGIN

template <typename Buffer>
class packer 
{
  typedef Buffer buffer_type;
  typedef msgpack::packer<typename buffer_type::buffer_implementation>
      packer_type;

  packer_type packer_;

public:
  packer (buffer_type& buffer)
    : packer_ (&buffer.impl ())
  {
  }

#if YAMAIL_CPP >= 11
  template <typename T>
  void pack (T&& t) { packer_.template pack (std::forward<T> (t)); }
#else
  template <typename T>
  void pack (T const& t) { packer_.template pack (t); }
#endif
};

YAMAIL_FQNS_DATA_MSGPACK_END
#endif // _YAMAIL_DATA_MSGPACK_PACKER_H_

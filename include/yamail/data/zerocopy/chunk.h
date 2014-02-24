#ifndef _YAMAIL_DATA_ZEROCOPY_CHUNK_H_
#define _YAMAIL_DATA_ZEROCOPY_CHUNK_H_

#include <yamail/config.h>
#include <yamail/data/zerocopy/namespace.h>

#include <yamail/compat/shared_ptr.h>

#include <utility> // std::pair
#include <cstddef> // std::size_t


YAMAIL_FQNS_DATA_ZC_BEGIN

class base_chunk
{
public:
  typedef char byte_t;
  virtual std::pair<byte_t*, std::size_t> buff() = 0;
  virtual ~base_chunk() {}
};

class const_base_chunk
{
public:
  typedef char byte_t;
  virtual std::pair<const byte_t*, std::size_t> buff() = 0;
  virtual ~const_base_chunk() {}
};

typedef compat::shared_ptr<base_chunk> base_chunk_ptr;
typedef compat::shared_ptr<const_base_chunk> const_base_chunk_ptr;

YAMAIL_FQNS_DATA_ZC_END

#endif // _YAMAIL_DATA_ZEROCOPY_CHUNK_H_

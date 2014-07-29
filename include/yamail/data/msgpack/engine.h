#ifndef _YAMAIL_DATA_MSGPACK_ENGINE_H_
#define _YAMAIL_DATA_MSGPACK_ENGINE_H_

#include <yamail/config.h>
#include <yamail/data/msgpack/namespace.h>

YAMAIL_FQNS_DATA_MSGPACK_BEGIN

// Traits type to select copy or noncopy semantic in compile time
// depending on 'Tag'.
struct engine
{
  template <typename Tag>
  struct buffer { typedef buffer<Tag> type; };

  template <typename Buffer>
  struct packer { typedef packer<Buffer> type; };

  struct unpacker { typedef unpacker type; };
  struct unpacked { typedef msgpack::unpacked type; };
};

YAMAIL_FQNS_DATA_MSGPACK_END
#endif // _YAMAIL_DATA_MSGPACK_ENGINE_H_

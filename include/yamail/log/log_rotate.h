#ifndef _YAMAIL_LOG_LOG_ROTATE_H_
#define _YAMAIL_LOG_LOG_ROTATE_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>

#include <list>

YAMAIL_NS_BEGIN
YAMAIL_NS_LOG_BEGIN

class sink_support_rotate {
public:
  virtual ~sink_support_rotate () {}
  virtual void rotate() = 0;
};
typedef sink_support_rotate* sink_support_rotate_ptr;

class log_rotate_manager
{
public:
  static log_rotate_manager& instance();

  void register_sink(sink_support_rotate_ptr sink);
  void unregister_sink(sink_support_rotate_ptr sink);
  void do_rotate();

private:
  typedef std::list<sink_support_rotate_ptr> sinks_list;
  sinks_list sinks_;
};

YAMAIL_NS_LOG_END
YAMAIL_NS_END

#endif // _YAMAIL_LOG_LOG_ROTATE_H_

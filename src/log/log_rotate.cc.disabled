#include <yamail/log/log_rotate.h>

YAMAIL_NS_BEGIN
YAMAIL_NS_LOG_BEGIN

log_rotate_manager log_rotater;

log_rotate_manager& log_rotate_manager::instance()
{
    return log_rotater;
}

void log_rotate_manager::register_sink(sink_support_rotate_ptr sink)
{
    sinks_.push_back(sink);
}

void log_rotate_manager::unregister_sink(sink_support_rotate_ptr sink)
{
    sinks_.remove(sink);
}

void log_rotate_manager::do_rotate()
{
    for (sinks_list::iterator i = sinks_.begin(), i_end = sinks_.end();
            i != i_end; ++i)
    {
        (*i)->rotate();
    }
}

YAMAIL_NS_LOG_END
YAMAIL_NS_END

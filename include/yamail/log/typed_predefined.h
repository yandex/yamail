#ifndef _YAMAIL_LOG_TYPED_PREDEFINED_H_
#define _YAMAIL_LOG_TYPED_PREDEFINED_H_
#include <yamail/config.h>
#include <yamail/log/namespace.h>
#include <yamail/log/typed.h>

#include <yamail/compat/chrono.h>

#include <time.h> // timespec and co.

#include <boost/thread.hpp>

#if defined(HAVE_STD_CHRONO) && HAVE_STD_CHRONO
namespace std {
namespace chrono {

template <typename C, typename T, class Cl, class D>
std::basic_ostream<C,T>&
operator<< (std::basic_ostream<C,T>& os, time_point<Cl,D> const& tp)
{
	return os << "(time_point)";
}

} // namespace chrono
}
#endif

#if defined(GENERATING_DOCUMENTATION)
namespace yamail { namespace log {
#else
YAMAIL_FQNS_LOG_BEGIN
#endif // GENERATING_DOCUMENTATION

namespace typed {

namespace detail {

struct time_field_helper 
{
  template <typename Clock, typename Duration>
  field_type
  operator() (YAMAIL_FQNS_COMPAT::chrono::time_point<Clock, Duration> time)
  const
  {
    return make_field (arg_time, time);
  }

  template <typename Clock, typename SysClock>
  field_type operator() (time_t time, long nanoseconds = 0L) const
  {
    typedef Clock clock_type;

    static const typename Clock::time_point    dst_now = Clock::now ();
    static const typename SysClock::time_point sys_now = SysClock::now ();

    return make_field (arg_time,
        SysClock::from_time_t (time) 
            - sys_now + dst_now 
            + YAMAIL_FQNS_COMPAT::chrono::nanoseconds (nanoseconds)
    );
  }

  field_type operator() (time_t time, long nanoseconds = 0L) const
  {
    return this->operator() <
                YAMAIL_FQNS_COMPAT::chrono::high_resolution_clock
              , YAMAIL_FQNS_COMPAT::chrono::system_clock
     > (time, nanoseconds);
  }

  field_type operator() (struct timeval const& tv) const
  {
    return (*this) (tv.tv_sec, 1000L * tv.tv_usec);
  }

  field_type operator() (struct timespec const& ts) const
  {
    return (*this) (ts.tv_sec, ts.tv_nsec);
  }
}; // time_field_helper

struct pid_field_helper 
{
	field_type operator() (::pid_t pid) const
	{
		return make_field (arg_pid, pid);
  }
};

struct ppid_field_helper
{
  field_type operator() (::pid_t ppid) const
  {
		return make_field (arg_ppid, ppid);
  }
};

struct tid_field_helper
{
	field_type operator() (boost::thread::id tid) const
	{
		return make_field (arg_tid, tid);
  }
};

struct service_field_helper
{
	field_type operator() (std::string const& service) const
	{
		return make_field (arg_tid, service);
  }
};

} // namespace detail

inline attributes_map&
operator<< (attributes_map& map, detail::time_field_helper const& time)
{
	return map 
	    << time (YAMAIL_FQNS_COMPAT::chrono::high_resolution_clock::now ());
}

inline attributes_map&
operator<< (attributes_map& map, detail::pid_field_helper const& pid)
{
  return map << pid (::getpid ());
}

inline attributes_map&
operator<< (attributes_map& map, detail::ppid_field_helper const& ppid)
{
  return map << ppid (::getppid ());
}

inline attributes_map&
operator<< (attributes_map& map, detail::tid_field_helper const& tid)
{
  return map << tid (boost::this_thread::get_id ());
}

inline attributes_map&
operator<< (attributes_map& map, detail::service_field_helper const& srv)
{
	// TODO: get service name somehow (from proccess name?)
  return map << srv ("(unknown service)");
}

namespace {
const detail::time_field_helper time_field = detail::time_field_helper ();
const detail::pid_field_helper pid_field = detail::pid_field_helper ();
const detail::ppid_field_helper ppid_field = detail::ppid_field_helper ();
const detail::tid_field_helper tid_field = detail::tid_field_helper ();
const detail::service_field_helper service_field = 
    detail::service_field_helper ();
}


} // namespace typed
#if defined(GENERATING_DOCUMENTATION)
}} // namespace yamail::log
#else
YAMAIL_FQNS_LOG_END
#endif // GENERATING_DOCUMENTATION

#endif // _YAMAIL_LOG_TYPED_PREDEFINED_H_
